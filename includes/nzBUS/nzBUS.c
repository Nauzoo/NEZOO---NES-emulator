#include "sizebits.h"
#include <stdbool.h>
#include "nzBUS.h"
//#include "nz6502.h"
// comemtario daora kkkk

void busWrite(struct BUS * bus, Word address, Byte data) {
    if (address >= 0x0000 && address <=  0xFFFF)
        bus->RAM[address] = data;
}
Byte busRead(struct BUS * bus, Word address, bool isReadOnly) {
    if (address >= 0x0000 && address <=  0xFFFF)
        return bus->RAM[address];
    return 0x00;
}