#include "nzBUS.h"
#include "sizebits.h"
#include <stdbool.h>
#include "nz6502.h"

BUS * myBus;
CPU * myCpu;
bool lIMP;

void cpuCreate(CPU *cpu) {
    myCpu = cpu;

    // initialize the registers with zeros
    // this is done to avoid null references
    myCpu->PgCount = 0x0000;
    myCpu->StkPtr = 0x00;
    myCpu->A = 0x00;
    myCpu->X = 0x00;
    myCpu->Y = 0x00;

    myCpu->StaReg = 0x00;

    myCpu->flag_C = (1<<0);
    myCpu->flag_Z = (1<<1);
    myCpu->flag_I = (1<<2);
    myCpu->flag_D = (1<<3);
    myCpu->flag_B = (1<<4);
    myCpu->flag_U = (1<<5);
    myCpu->flag_V = (1<<6);
    myCpu->flag_N = (1<<7);

    myCpu->fetched = 0x00;
    myCpu->abs_address = 0x0000;
    myCpu->opcode = 0x00;
    myCpu->cycles = 0;

    lIMP = false;
}


// set the bus pointer to the bus "instance"
void cpuConBus(BUS * bus){
    myBus = bus;
}

// write the data from a specific stack location
void cpuWrite(Word address, Byte data){
    busWrite(myBus, address, data);
}

// read the data from a specific stack location
Byte cpuRead(Word address, bool isReadOnly){
    return busRead(myBus, address, isReadOnly);
}

// gets the next instruction pointed by the program counter
Byte cpuFetch(){
    if (!lIMP) {
        myCpu->fetched = cpuRead(myCpu->abs_address, false);
    }
    return myCpu->fetched;
}



// returns one if a flag is active
Byte cpuIsActiveFlag(Byte flag) {
    return (myCpu->StaReg & flag) > 0 ? 1 : 0;
}

void cpuSetStaFlag(Byte flag, bool value) {
    if (value) {
        myCpu->StaReg |= flag;
    } else {
        myCpu->StaReg &= ~flag;
    }
}

void cpuReset() {
    // delete the data from the respective resgisters:
    myCpu->A = 0;
    myCpu->X = 0;
    myCpu->Y = 0;

    // sets the stack pointer to a known location
    myCpu->StkPtr = 0xFD;
    // clean the satus register
    myCpu->StaReg = 0x00;


    // sets the program counter to a start position

    // the program counter holds a 16 (2 bytes) bit adress, and the cpu bus can only transfer
    // 8 bits (1 byte) at a time.
    // the transfer needs to be done in 2 steps:

    myCpu->abs_address = 0xFFFC;

    // 0XFFFC: 00010010
    // 0XFFFD: 10100010
    // (values are just an example)

    Word lowByte = (Word) cpuRead(myCpu->abs_address + 0,false);
    //lowByte (0xFFFC) = (00000000)00010010

    Word higByte = (Word) cpuRead(myCpu->abs_address + 1,false);
    //higByte (0xFFFD) = (00000000)10100010

    myCpu->PgCount = (higByte << 8) | lowByte;
    //higByte << 8 : 1010001000000000

    // 1010001000000000
    // | (or)
    // 0000000000010010
    // ----------------
    // 1010001000010010
    //
    // the or operator "glue toghter" the two parts of the address


    myCpu->abs_address = 0x0000;
    myCpu->rel_address = 0x0000;

    myCpu->fetched = 0x00;

    // reset takes 8 cycles
    myCpu->cycles = 8;
}

void _interrupt() {

    // template function for interrupt request and non maskable interrupt
    // saves the states of the stack pointer, status register and the program counter

	cpuWrite(0x0100 + myCpu->StkPtr, (myCpu->PgCount >> 8) & 0x00FF);
	myCpu->StkPtr--;
    cpuWrite(0x0100 + myCpu->StkPtr, myCpu->PgCount & 0x00FF);
	myCpu->StkPtr--;


    // sets the interrupt flag on
    cpuSetStaFlag(myCpu->flag_B, 0);
    cpuSetStaFlag(myCpu->flag_U, 1);
    cpuSetStaFlag(myCpu->flag_I, 1);

	cpuWrite(0x0100 + myCpu->StkPtr, myCpu->StaReg);
	myCpu->StkPtr--;

	// read the new program counter location from fixed address, also done in two steps
	myCpu->abs_address = 0xFFFE;
	Word lowByte = (Word) cpuRead(myCpu->abs_address + 0, false);
	Word higByte = (Word) cpuRead(myCpu->abs_address + 1, false);
	myCpu->PgCount = (higByte << 8) | lowByte;
}


void cpuIntReq() {
    if (~cpuIsActiveFlag(myCpu->flag_I)) {
        _interrupt();
    }

    // interrupt request takes 7 cycles
    myCpu->cycles = 7;
}

void cpuNonMaskInt() {
    _interrupt();

    // non maskable interrupt takes 8 cycles
    myCpu->cycles = 8;
}


// addressing modes:

void cpuIMPAddMod() {
    myCpu->fetched = myCpu->A;
    lIMP = true;
}

void cpuIMMAddMod() {
    myCpu->PgCount++;
    myCpu->abs_address = myCpu->PgCount;
    lIMP = false;
}

void cpuZP0AddMod() {
    myCpu->abs_address = cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;

    // gets the current program counter value and masks with:
    myCpu->abs_address &= 0x00FF;

    lIMP = false;
}

void cpuZPXAddMod() {
    myCpu->abs_address = (cpuRead(myCpu->PgCount, false) + myCpu->X);
    myCpu->PgCount++;

    // gets the current program counter value and masks with:
    myCpu->abs_address &= 0x00FF;

    lIMP = false;
}

void cpuZPYAddMod() {
    myCpu->abs_address = (cpuRead(myCpu->PgCount, false) + myCpu->Y);
    myCpu->PgCount++;

    // gets the current program counter value and masks with:
    myCpu->abs_address &= 0x00FF;

    lIMP = false;
}


void cpuABSAddMod(){
    Word lowByte = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;
    Word higByte = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;

    myCpu->abs_address = (higByte << 8) | lowByte;

    lIMP = false;
}

void cpuABXAddMod(){
    Word lowByte = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;
    Word higByte = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;

    myCpu->abs_address = (higByte << 8) | lowByte;
    myCpu->abs_address += myCpu->X;

    if ((myCpu->abs_address & 0xFF00) != higByte << 8){
        myCpu->cycles += 1;
    } // possible else -> cycles += 0

    lIMP = false;
}

void cpuABYAddMod(){
    Word lowByte = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;
    Word higByte = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;

    myCpu->abs_address = (higByte << 8) | lowByte;
    myCpu->abs_address += myCpu->Y;

    if ((myCpu->abs_address & 0xFF00) != (higByte << 8)) {
		myCpu->cycles += 1;
    } // possible else ->cycles += 0

    lIMP = false;
}

void cpuINDAddMod(){
    Word ptrLow = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;
    Word ptrHig = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;

    Word pointer = (ptrHig << 8) | ptrLow;

    // this statement is made to replicate a 6502's hardware bug. check more at: nessdev wiki
    if (ptrLow == 0x00FF) {
        myCpu->abs_address = (cpuRead(pointer + 0xFF00, false) << 8) | cpuRead(pointer, false);
    } else {
        myCpu->abs_address = (cpuRead(pointer + 1, false) << 8) | cpuRead(pointer, false);
    }

    lIMP = false;
}

void cpuIZXAddMod() {
    Word add = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;

    Word Xadd = (Word) myCpu->X;
    Word lowByte = cpuRead((add + Xadd) & 0x00FF, false);
    Word higByte = cpuRead((add + Xadd + 1) & 0x00FF, false);

    myCpu->abs_address = (higByte << 8) | lowByte;

    lIMP = false;
}

void cpuIZYAddMod() {
    Word add = (Word) cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;

    Word lowByte = cpuRead(add & 0x00FF, false);
    Word higByte = cpuRead((add + 1) & 0x00FF, false);

    myCpu->abs_address = (higByte << 8) | lowByte;
    myCpu->abs_address += myCpu->Y;

    if ((myCpu->abs_address & 0xFF00) != higByte << 8){
        myCpu->cycles += 1;
    } // possible else -> cycles += 0;

    lIMP = false;
}

void cpuRELAddMod() {
    myCpu->rel_address = cpuRead(myCpu->PgCount, false);
    myCpu->PgCount++;
    if (myCpu->rel_address & 0x80) {
        myCpu->rel_address |= 0xFF00;
    }

    lIMP = false;
}

//  executes the code? I guess.
void cpuExecute() {
    if (myCpu->cycles == 0){

        Byte opcode = cpuRead(myCpu->PgCount, false);

        switch (opcode) {
            // ADC
            case 0x69:
                myCpu->cycles = 2; // sets the cpu cycles
                cpuIMMAddMod();    // calls adressing mode
                cpuInsADC();       // calls instruction
                break;
            case 0x65: myCpu->cycles = 3; cpuZP0AddMod(); cpuInsADC(); break;
            case 0x75: myCpu->cycles = 4; cpuZPXAddMod(); cpuInsADC(); break;
            case 0x6D: myCpu->cycles = 4; cpuABSAddMod(); cpuInsADC(); break;
            case 0x7D: myCpu->cycles = 4; cpuABXAddMod(); cpuInsADC(); break;
            case 0x79: myCpu->cycles = 4; cpuABYAddMod(); cpuInsADC(); break;
            case 0x61: myCpu->cycles = 6; cpuIZXAddMod(); cpuInsADC(); break;
            case 0x71: myCpu->cycles = 5; cpuIZYAddMod(); cpuInsADC(); break;
            // AND
            case 0x29: myCpu->cycles = 2; cpuIMMAddMod(); cpuInsAND(); break;
            case 0x25: myCpu->cycles = 3; cpuZP0AddMod(); cpuInsAND(); break;
            case 0x35: myCpu->cycles = 4; cpuZPXAddMod(); cpuInsAND(); break;
            case 0x2D: myCpu->cycles = 4; cpuABSAddMod(); cpuInsAND(); break;
            case 0x3D: myCpu->cycles = 4; cpuABXAddMod(); cpuInsAND(); break;
            case 0x39: myCpu->cycles = 4; cpuABYAddMod(); cpuInsAND(); break;
            case 0x21: myCpu->cycles = 6; cpuIZXAddMod(); cpuInsAND(); break;
            case 0x31: myCpu->cycles = 5; cpuIZYAddMod(); cpuInsAND(); break;
            //ASL
            case 0x0A: myCpu->cycles = 2; cpuIMPAddMod(); cpuInsASL(); break;
            case 0x06: myCpu->cycles = 5; cpuZP0AddMod(); cpuInsASL(); break;
            case 0x16: myCpu->cycles = 6; cpuZPXAddMod(); cpuInsASL(); break;
            case 0x0E: myCpu->cycles = 6; cpuABSAddMod(); cpuInsASL(); break;
            case 0x1E: myCpu->cycles = 7; cpuABXAddMod(); cpuInsASL(); break;
            // BCC
            case 0x90: myCpu->cycles = 2; cpuRELAddMod(); cpuInsBCC(); break;
            //BCS
            case 0xB0: myCpu->cycles = 2; cpuRELAddMod(); cpuInsBCS(); break;
            // BEQ
            case 0xF0: myCpu->cycles = 2; cpuRELAddMod(); cpuInsBEQ(); break;
            // BIT
            case 0x24: myCpu->cycles = 3; cpuZP0AddMod(); cpuInsBIT(); break;
            case 0x2C: myCpu->cycles = 4; cpuABSAddMod(); cpuInsBIT(); break;
            // BMI
            case 0x30: myCpu->cycles = 2; cpuRELAddMod(); cpuInsBMI(); break;
            // BNE
            case 0xD0: myCpu->cycles = 2; cpuRELAddMod(); cpuInsBNE(); break;
            // BPL
            case 0x10: myCpu->cycles = 2; cpuRELAddMod(); cpuInsBPL(); break;
            // BRK
            case 0x00: myCpu->cycles = 7; cpuIMPAddMod(); cpuInsBRK(); break;



            default: break;
        }
        cpuSetStaFlag(myCpu->flag_U, true);
        myCpu->PgCount++;
    }
    myCpu->cycles--;
}


// instructions:

void cpuInsADC() {
    // This instruction adds the contents of a memory location to the accumulator
    // together with the carry bit.
    //If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.

    // gets the value at the program counter location
    cpuFetch();
    // add the value to the accumulator toghter with the carry bits
    Word buff = (Word)myCpu->A + (Word)myCpu->fetched + (Word)cpuIsActiveFlag(myCpu->flag_C);
    // set the corresponding flags in case of:
    cpuSetStaFlag(myCpu->flag_C, buff > 255);           // CARRY
    cpuSetStaFlag(myCpu->flag_Z, (buff & 0x00FF) == 0); // ZERO
    cpuSetStaFlag(myCpu->flag_N, buff & 0x80);          // NEGATIVE
    // the negative flag is set if the most significant bit of the result is one.

    // the overflow flag is set if the most significant bit of the following situations are:
    // POSITIVE + POSITIVE = 1
    // NEGATIVE + NEGATIVE = 0

    // the most significant bit set to 1 implies that the value is negative, so, if u add up
    // two positives and end up with 1 in the most significant bit, your value will be treated
    // as a negative. In that case, your value has "wrapped around", an overflow!

    // to check a overflow situation, the formula:
    // [(Acumulator ^ MemoryValue) & ~(Acumulator ^ result)] & 0x0080
    // is applied.
    bool OverFlow = (~((Word)myCpu->A ^ (Word)myCpu->fetched) & ((Word)myCpu->A ^ buff)) & 0x0080;
    cpuSetStaFlag(myCpu->flag_V, OverFlow);

    myCpu->A = buff & 0x00FF;

    // This instruction may require an extra clock cycle
    myCpu->cycles += 1;

}


void cpuInsAND() {
    cpuFetch();
	myCpu->A &= myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == 0x00); // if the Acumulator == 0
    cpuSetStaFlag(myCpu->flag_N, myCpu->A & 0x80);  // if bit 7 is set

    myCpu->cycles += 1;
}

void cpuInsASL() {
    cpuFetch();

    Word holder = (Word)(myCpu->fetched) << 1;

    cpuSetStaFlag(myCpu->flag_C, (holder & 0xFF00) > 0);
    cpuSetStaFlag(myCpu->flag_Z, (holder & 0x00FF) == 0x00);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

    if(lIMP){
        myCpu->A = holder & 0x00FF;
    } else {
        cpuWrite(myCpu->abs_address, holder & 0x00FF);
    }
}

void cpuInsBCC() {
    if (~cpuIsActiveFlag(myCpu->flag_C)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}

void cpuInsBCS() {
    if (cpuIsActiveFlag(myCpu->flag_C)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}


void cpuInsBEQ() {
    if (cpuIsActiveFlag(myCpu->flag_Z)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}

void cpuInsBIT() {
    cpuFetch();

    Byte bitTest = myCpu->A & myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_Z, bitTest == 0);
    cpuSetStaFlag(myCpu->flag_V, myCpu->fetched & (1 << 6));
    cpuSetStaFlag(myCpu->flag_N, myCpu->fetched & (1 << 7));
}

void cpuInsBMI() {
    if (cpuIsActiveFlag(myCpu->flag_N)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}

void cpuInsBNE() {
    if (~cpuIsActiveFlag(myCpu->flag_Z)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}

void cpuInsBPL() {
    if (~cpuIsActiveFlag(myCpu->flag_N)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}

void cpuInsBRK() {
    myCpu->PgCount++;
    _interrupt();
}

void cpuInsBVC() {
    if (~cpuIsActiveFlag(myCpu->flag_V)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}

void cpuInsBVS() {
    if (cpuIsActiveFlag(myCpu->flag_V)) {

        myCpu->cycles++;

        myCpu->abs_address = myCpu->PgCount + myCpu->rel_address;
        if ((myCpu->PgCount & 0xFF00) != (myCpu->abs_address & 0xFF00))
            myCpu->cycles++;

        myCpu->PgCount = myCpu->abs_address;
    }
}

void cpuInsCLC() {
    cpuSetStaFlag(myCpu->flag_C, false);
}

void cpuInsCLD() {
    cpuSetStaFlag(myCpu->flag_D, false);
}

void cpuInsCLI() {
    cpuSetStaFlag(myCpu->flag_I, false);
}

void cpuInsCLV() {
    cpuSetStaFlag(myCpu->flag_V, false);
}

void cpuInsCMP() {
    cpuFetch();

    Byte holder = myCpu->A - myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_C, myCpu->A >= myCpu->fetched);
    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == myCpu->fetched);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

    myCpu->cycles += 1;
}

void cpuInsCPX() {
    cpuFetch();

    Byte holder = myCpu->X - myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_C, myCpu->X >= myCpu->fetched);
    cpuSetStaFlag(myCpu->flag_Z, myCpu->X == myCpu->fetched);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

}

void cpuInsCPY() {
    cpuFetch();

    Byte holder = myCpu->Y - myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_C, myCpu->Y >= myCpu->fetched);
    cpuSetStaFlag(myCpu->flag_Z, myCpu->Y == myCpu->fetched);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

}

void cpuInsDEC() {
    cpuFetch();

    Byte holder = myCpu->fetched - 1;
    cpuSetStaFlag(myCpu->flag_Z, holder == 0);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

    cpuWrite(myCpu->abs_address, holder);
}


void cpuInsDEX() {

    myCpu->X--;
    cpuSetStaFlag(myCpu->flag_Z, myCpu->X == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->Y & 0x80);

   }

void cpuInsDEY() {

    myCpu->Y--;
    cpuSetStaFlag(myCpu->flag_Z, myCpu->Y == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->Y & 0x80);

}

void cpuInsEOR() {
    cpuFetch();
	myCpu->A ^= myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == 0x00); // if the Acumulator == 0
    cpuSetStaFlag(myCpu->flag_N, myCpu->A & 0x80);  // if bit 7 is set

    myCpu->cycles += 1;
}

void cpuInsINC() {
    cpuFetch();

    Byte holder = myCpu->fetched + 1;
    cpuSetStaFlag(myCpu->flag_Z, holder == 0);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

    cpuWrite(myCpu->abs_address, holder);
}

void cpuInsINX() {

    myCpu->X++;
    cpuSetStaFlag(myCpu->flag_Z, myCpu->X == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->Y & 0x80);

   }

void cpuInsINY() {

    myCpu->Y++;
    cpuSetStaFlag(myCpu->flag_Z, myCpu->Y == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->Y & 0x80);

}

void cpuInsJMP() {
    myCpu->PgCount = myCpu->abs_address;

}

void cpuInsJSR() {
    myCpu->PgCount--;

    cpuWrite(0x0100 + myCpu->StkPtr, (myCpu->PgCount >> 8) & 0x00FF);
    myCpu->StkPtr--;
    cpuWrite(0x0100 + myCpu->StkPtr, myCpu->PgCount & 0x00FF);
    myCpu->StkPtr--;

    myCpu->PgCount = myCpu->abs_address;
}

void cpuInsLDA() {
    cpuFetch();

    myCpu->A = myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->A & 0x80);

    myCpu->cycles += 1;
}

void cpuInsLDX() {
    cpuFetch();

    myCpu->X = myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->X == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->X & 0x80);

    myCpu->cycles += 1;
}

void cpuInsLDY() {
    cpuFetch();

    myCpu->Y = myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->Y == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->Y & 0x80);

    myCpu->cycles += 1;
}

void cpuInsLSR() {
    cpuFetch();

    cpuSetStaFlag(myCpu->flag_C, myCpu->fetched & 0x0001);

    Word holder = (Word)(myCpu->fetched) >> 1;

    cpuSetStaFlag(myCpu->flag_Z, (holder & 0x00FF) == 0x00);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

    if(lIMP){
        myCpu->A = holder & 0x00FF;
    } else {
        cpuWrite(myCpu->abs_address, holder & 0x00FF);
    }
}

void cpuInsNOP() {
   //this instruction does nothing, lol.
}

void cpuInsORA() {
    cpuFetch();
	myCpu->A |= myCpu->fetched;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == 0x00); // if the Acumulator == 0
    cpuSetStaFlag(myCpu->flag_N, myCpu->A & 0x80);  // if bit 7 is set

    myCpu->cycles += 1;

}

void cpuInsPHA() {
    cpuWrite(0x0100 + myCpu->StkPtr, myCpu->A);
    myCpu->StkPtr--;
}

void cpuInsPHP() {
    cpuWrite(0x0100 + myCpu->StkPtr, myCpu->StaReg);
    myCpu->StkPtr--;
}

void cpuInsPLA() {
    myCpu->StkPtr++;
    myCpu->A = cpuRead(0x0100 + myCpu->StkPtr, false);

    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == 0x00);
    cpuSetStaFlag(myCpu->flag_N, myCpu->A & 0x80);
}

void cpuInsPLP() {
    myCpu->StkPtr++;
    myCpu->StaReg = cpuRead(0x0100 + myCpu->StkPtr, false);
    cpuSetStaFlag(myCpu->flag_U, true);

}

void cpuInsROL() {
    cpuFetch();

    Byte holder = myCpu->fetched << 1;
    holder |= (Byte)cpuIsActiveFlag(myCpu->flag_C);

    cpuSetStaFlag(myCpu->flag_C, myCpu->fetched & 0x80);
    cpuSetStaFlag(myCpu->flag_Z, holder == 0);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

    if (lIMP) {
        myCpu->A = holder;
    } else {
        cpuWrite(myCpu->abs_address, holder);
    }
}

void cpuInsROR() {
    cpuFetch();

    Byte holder = myCpu->fetched >> 1;
    holder |= (Byte)cpuIsActiveFlag(myCpu->flag_C) << 7;

    cpuSetStaFlag(myCpu->flag_C, myCpu->fetched & 0x01);
    cpuSetStaFlag(myCpu->flag_Z, holder == 0);
    cpuSetStaFlag(myCpu->flag_N, holder & 0x80);

    if (lIMP) {
        myCpu->A = holder;
    } else {
        cpuWrite(myCpu->abs_address, holder);
    }

}

void cpuInsRTI() {
    // restores the cpu to the state before the interrupt

    // loads the status register
    myCpu->StkPtr++;
    myCpu->StaReg = cpuRead(0x0100 + myCpu->StkPtr, false);
    myCpu->StaReg &= ~myCpu->flag_B;
    myCpu->StaReg &= ~myCpu->flag_U;

    // loads the program counter
    myCpu->StkPtr++;
    myCpu->PgCount = (Word) cpuRead(0x0100 + myCpu->StkPtr, false);
    myCpu->StkPtr++;
    myCpu->PgCount |= (Word) cpuRead(0x0100 + myCpu->StkPtr,false) << 8;

}

void cpuInsRTS() {
    myCpu->StkPtr++;
    myCpu->PgCount = (Word)cpuRead(0x0100 + myCpu->StkPtr, false);
    myCpu->StkPtr++;
    myCpu->PgCount |= (Word)(Word)cpuRead(0x0100 + myCpu->StkPtr, false) << 8;

    myCpu->PgCount++;
}


void cpuInsSBC() {
    // This instruction subtracts the contents of a memory location to the accumulator
    // together with the not of the carry bit. If overflow occurs the carry bit is clear,
    // this enables multiple byte subtraction to be performed.

    cpuFetch();

    // To perform the subtraction operation the cpu uses the same hardware as the add operations.
    // the following formula is used in this case:
    // Acumulator + (-fetched) + 1 + carry
    // two have a negative value of the fetched number, the "two's complement" is used.
    // to get the two's complement: invert all the bits and add 1 ignoring overflows.

    Word flipFet = ((Word)myCpu->fetched) & 0x00FF; // get all the bits flipped

    Word buff = (Word)myCpu->A + flipFet + (Word)cpuIsActiveFlag(myCpu->flag_C);

    cpuSetStaFlag(myCpu->flag_C, buff & 0xFF00);
    cpuSetStaFlag(myCpu->flag_Z, (buff & 0xFF00) == 0);
    cpuSetStaFlag(myCpu->flag_N, buff & 0x0080);


    cpuSetStaFlag(myCpu->flag_V, ((buff ^ (Word)myCpu->A) & (buff ^ flipFet)) & 0x0080);

    myCpu->A = buff & 0x00FF;

    myCpu->cycles += 1;

}

void cpuInsSEC(){
    cpuSetStaFlag(myCpu->flag_C, true);

}

void cpuInsSED(){
    cpuSetStaFlag(myCpu->flag_D, true);

}

void cpuInsSEI(){
    cpuSetStaFlag(myCpu->flag_I, true);

}

void cpuInsSTA(){
    cpuWrite(myCpu->abs_address, myCpu->A);
}

void cpuInsSTX(){
    cpuWrite(myCpu->abs_address, myCpu->X);
}

void cpuInsSTY(){
    cpuWrite(myCpu->abs_address, myCpu->Y);
}

void cpuInsTAX() {
    myCpu->X = myCpu->A;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->X == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->X & 0x80);
}

void cpuInsTAY() {
    myCpu->Y = myCpu->A;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->Y == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->Y & 0x80);
}

void cpuInsTSX() {
    myCpu->X = myCpu->StkPtr;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->X == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->X & 0x80);
}

void cpuInsTXA() {
    myCpu->A = myCpu->X;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->A & 0x80);
}

void cpuInsTXS() {
    myCpu->StkPtr = myCpu->X;
}

void cpuInsTYA() {
    myCpu->A = myCpu->Y;

    cpuSetStaFlag(myCpu->flag_Z, myCpu->A == 0);
    cpuSetStaFlag(myCpu->flag_N, myCpu->A & 0x80);
}

