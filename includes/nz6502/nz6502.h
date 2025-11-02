#ifndef NZ6502_H
#define  NZ6502_H

#include "sizebits.h"
#include "nzBUS.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

    char* name;
    void (*instruction)();
    void (*addrMode)();
    uint8_t cycles;

} Instruction;

typedef struct
{
    Word address;
    // For printing to the disassembler:
    char addrString[5];        // 00FF
    char opcode[3];            // A0
    char mneumonic[4];         // XXX
    char addrMode[5];          // IMM
    char data_argument[8];     // #$AB 
    char address_argument[14]; // $FF [$FFFF]

} InstructionMap;

typedef struct
{
    int size;
    InstructionMap* list;
} InstructionList;


typedef struct CPU {

    Word PgCount;    //[Reg.] Program counter (Points to the next instruction to be executed)
    Byte StkPtr;     //[Reg.] Stack pointer (Holds the location of the next free space on the stack)
    Byte A;          //[Reg.] Acumulator (Maths and logic operations + critical routines)
    Byte X;          //[Reg.] index register X (Hold counters and access memory + copy Stack pointer)
    Byte Y;          //[Reg.] index register Y (Hold counters and access memory)

    // Processor Status (Set flags related to the process current situation)

    Byte StaReg;

    Byte flag_C; //status flag carry               0b00000001
    Byte flag_Z; //status flag zero                0b00000010
    Byte flag_I; //status flag disable interrupts  0b00000100
    Byte flag_D; //status flag decimal mode        0b00001000
    Byte flag_B; //status flag break               0b00010000
    Byte flag_U; //status flag unused              0b00100000
    Byte flag_V; //status flag overflow            0b01000000
    Byte flag_N; //status flag negative            0b10000000

    // Convinience values:
    Byte fetched;            // stores the last value fetched from memory
    Word abs_address;        // stores the absolute address (full memory location) for an instruction
                             // its seted by the adressing modes
    signed char rel_address; // stores the local address within a page
                             // its seted by the adressing modes
    Byte opcode;             // stores the last instruction (binary)

    Byte cycles;      // stores the number of cycles for an operation

}CPU;



/** sets a pointer to the CPU strcuct for accessing the internal functions */
void cpuCreate(CPU * cpu);

/** sets a bus for the CPU communication */
void cpuConBus(BUS * bus);

/** writes an 8 bit data to the consoles RAM */
void cpuWrite(Word address, Byte data);

/** reads an 8 bit piece of data from the consoles RAM */
Byte cpuRead(Word address, bool isReadOnly);

/** fetches the next instruction pointed by the Program counter */
Byte cpuFetch();

/** executes an instruction, adding the code to the memory is needed **/
void cpuClock();

/** checks if a certain status register flag is activated */
bool cpuIsFlagActive(Byte flag);

/** sets a specific satus register flag to either 0 or 1 */
void cpuSetStaFlag(Byte flag, bool value);

/** Disassembles a region of memory into human readble code */
InstructionList* cupDisassemble(Word initAddr, Word finalAddr);

// Interrupt modes: stop the excecution of the code, finishing the current instruction first

/** sets the CPU in its start mode */
void cpuReset();

/** stops the process and excecute an specific instruction */
void cpuIntReq();

/** does the same as cpuIntReq but its stopped by the I flag */
void cpuNonMaskInt();


// Addressing modes:

// standards

/** IMPLICIT + A: the instruction has a defined address to operate */
void cpuIMPAddMod();

/** IMEDIATE: allows the proggrammer to specifify a constant to be used in the instruction*/
void cpuIMMAddMod();

/** ZERO PAGE ADDRESSING: looks specificly in page 0 (high byte in zero) */
void cpuZP0AddMod();

/** ABSOLUTE: operates with a supplied address */
void cpuABSAddMod();

/** RELATIVE: used for branching instructions, adds a (singned) 8 bits offset relative to the current program counter */
void cpuRELAddMod();

/** INDIRECT: works as a pointer (JMP) */
void cpuINDAddMod();

// indexeds

/** ZERO PAGE + X OFFSET: looks specificly in page 0 and adds the X register offset */
void cpuZPXAddMod();

/** ZERO PAGE + Y OFFSET: looks specificly in page 0 and adds the Y register offset */
void cpuZPYAddMod();

/** ABSOLUTE + X: operates with a supplied address + X offset */
void cpuABXAddMod();

/** ABSOLUTE + Y: operates with a supplied address + Y offset */
void cpuABYAddMod();

/** INDIRECT ZERO PAGE + X: works as a pointer in the 0 page + X offset */
void cpuIZXAddMod();
/** INDIRECT ZERO PAGE + Y: works as a pointer in the 0 page + Y offset */
void cpuIZYAddMod();


// Instructions:

/** instruction: Ilegal instruction */
void cpuInsXXX();

/** instruction: Add with carry **/
void cpuInsADC();

/** instruction: A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory **/
void cpuInsAND();

/** instruction: This operation shifts all the bits of the accumulator or memory contents one bit left. Bit 7 is placed in the carry flag. **/
void cpuInsASL();

/** instruction: If the carry flag is clear then add the relative displacement to the program counter to cause a branch to a new location. **/
void cpuInsBCC();

/** instruction: If the carry flag is set then add the relative displacement to the program counter to cause a branch to a new location. **/
void cpuInsBCS();

/** instruction: If the zero flag is set then add the relative displacement to the program counter to cause a branch to a new location.**/
void cpuInsBEQ();

/** instruction: This instructions is used to test if one or more bits are set in a target memory location. The mask pattern in A is ANDed with the value in memory to set or clear the zero flag, but the result is not kept. Bits 7 and 6 of the value from memory are copied into the N and V flags.**/
void cpuInsBIT();

/** instruction: If the negative flag is set then add the relative displacement to the program counter to cause a branch to a new location. **/
void cpuInsBMI();

/** instruction: If the zero flag is clear then add the relative displacement to the program counter to cause a branch to a new location. **/
void cpuInsBNE();

/** instruction: If the negative flag is clear then add the relative displacement to the program counter to cause a branch to a new location. **/
void cpuInsBPL();

/** instruction: The BRK instruction forces the generation of an interrupt request. The program counter and processor status are pushed on the stack then the IRQ interrupt vector at $FFFE/F is loaded into the PC and the break flag in the status set to one.**/
void cpuInsBRK();

/** instruction: If the overflow flag is clear then add the relative displacement to the program counter to cause a branch to a new location. **/
void cpuInsBVC();

/** instruction: If the overflow flag is set then add the relative displacement to the program counter to cause a branch to a new location. **/
void cpuInsBVS();

/** instruction: Sets the carry flag to 0 **/
void cpuInsCLC();

/** instruction: Sets the decimal flag to 0 **/
void cpuInsCLD();

/** instruction: Sets the interrupt disable flag to 0 **/
void cpuInsCLI();

/** instruction: Sets the overflow flag to 0 **/
void cpuInsCLV();

/** instruction: This instruction compares the contents of the accumulator with another memory held value and sets the zero and carry flags as appropriate.**/
void cpuInsCMP();

/** instruction: This instruction compares the contents of the X register with another memory held value and sets the zero and carry flags as appropriate. **/
void cpuInsCPX();

/** instruction: This instruction compares the contents of the Y register with another memory held value and sets the zero and carry flags as appropriate. **/
void cpuInsCPY();

/** instruction: Subtracts one from the value held at a specified memory location setting the zero and negative flags as appropriate. **/
void cpuInsDEC();

/** instruction: Subtracts one from the X register setting the zero and negative flags as appropriate. **/
void cpuInsDEX();

/** instruction: Subtracts one from the Y register setting the zero and negative flags as appropriate. **/
void cpuInsDEY();

/** instruction: An exclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory. **/
void cpuInsEOR();

/** instruction: Adds one to the value held at a specified memory location setting the zero and negative flags as appropriate.**/
void cpuInsINC();

/** instruction: Adds one to the X register setting the zero and negative flags as appropriate.**/
void cpuInsINX();

/** instruction: Adds one to the Y register setting the zero and negative flags as appropriate.**/
void cpuInsINY();

/** instruction: Sets the program counter to the address specified by the operand.**/
void cpuInsJMP();

/** instruction: The JSR instruction pushes the address (minus one) of the return point on to the stack and then sets the program counter to the target memory address.**/
void cpuInsJSR();

/** instruction: Loads a byte of memory into the accumulator setting the zero and negative flags as appropriate.**/
void cpuInsLDA();

/** instruction: Loads a byte of memory into the X register setting the zero and negative flags as appropriate.**/
void cpuInsLDX();

/** instruction: Loads a byte of memory into the Y register setting the zero and negative flags as appropriate.**/
void cpuInsLDY();

/** instruction: Each of the bits in A or M is shift one place to the right. The bit that was in bit 0 is shifted into the carry flag. Bit 7 is set to zero.**/
void cpuInsLSR();

/** instruction: The NOP instruction causes no changes to the processor other than the normal incrementing of the program counter to the next instruction**/
void cpuInsNOP();

/** instruction: An inclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.**/
void cpuInsORA();

/** instruction: Pushes a copy of the accumulator on to the stack.**/
void cpuInsPHA();

/** instruction: Pushes a copy of the status flags on to the stack.**/
void cpuInsPHP();

/** instruction: Pulls a value from the stack into the accumulator. The Z and N flags are set as appropriate **/
void cpuInsPLA();

/** instruction: Pulls an 8 bit value from the stack and into the processor flags. The flags will take on new states as determined by the value pulled.**/
void cpuInsPLP();

/** instruction: Move each of the bits in either A or M one place to the left. Bit 0 is filled with the current value of the carry flag whilst the old bit 7 becomes the new carry flag value.**/
void cpuInsROL();

/** instruction: Move each of the bits in either A or M one place to the right. Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes the new carry flag value.**/
void cpuInsROR();

// return from Interrupt
/** instruction: restores the cpu to the state before the interruption */
void cpuInsRTI();

/** instruction: The RTS instruction is used at the end of a subroutine to return to the calling routine. It pulls the program counter (minus one) from the stack.**/
void cpuInsRTS();

/** instruction: Subtract with carry **/
void cpuInsSBC();

/** instruction: Set the carry flag to one.**/
void cpuInsSEC();

/** instruction: Set the decimal flag to one.**/
void cpuInsSED();

/** instruction: Set the interrupt flag to one.**/
void cpuInsSEI();

/** instruction: Stores the contents of the accumulator into memory.**/
void cpuInsSTA();

/** instruction: Stores the contents of the X register into memory.**/
void cpuInsSTX();

/** instruction: Stores the contents of the Y register into memory.**/
void cpuInsSTY();

/** instruction: Copies the current contents of the accumulator into the X register and sets the zero and negative flags as appropriate.**/
void cpuInsTAX();

/** instruction: Copies the current contents of the accumulator into the Y register and sets the zero and negative flags as appropriate.**/
void cpuInsTAY();

/** instruction: Copies the current contents of the stack pointer into the X register and sets the zero and negative flags as appropriate.**/
void cpuInsTSX();

/** instruction: Copies the current contents of the X register into the accumulator and sets the zero and negative flags as appropriate.**/
void cpuInsTXA();

/** instruction: Copies the current contents of the X register into the Stack pointer.**/
void cpuInsTXS();

/** instruction: Copies the current contents of the Y register into the accumulator and sets the zero and negative flags as appropriate.**/
void cpuInsTYA();

#ifdef __cplusplus
}
#endif
#endif