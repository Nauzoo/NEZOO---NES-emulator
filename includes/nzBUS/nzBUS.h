#ifndef NZBUS_H
#define NZBUS_H

#include <stdbool.h>
#include "sizebits.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RAM_SIZE 65536

typedef struct BUS {

    // Devices:
        // Emulated RAM
    // memorySize = 64 * 1024 = 64kb
    Byte RAM[RAM_SIZE];

} BUS;

/** writes an 8 bit data to the consoles RAM */
void busWrite(BUS * bus, Word address, Byte data);

/** reads an 8 bit piece of data from the consoles RAM */
Byte busRead(BUS * bus, Word address, bool isReadOnly);


#ifdef __cplusplus
}
#endif

#endif