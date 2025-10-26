
#include <stdbool.h>
#include <stdio.h>
#include "nz6502.h"
#include "nzBUS.h"
#include "sizebits.h"


int main(int argc, char *argv[])
{
    CPU newCpu;
    BUS newBus;

    cpuCreate(&newCpu);
    cpuConBus(&newBus);

    cpuReset();

    return 0;
}
