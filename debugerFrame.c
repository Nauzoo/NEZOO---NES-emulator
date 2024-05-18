#include <curses.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include "nz6502.h"
#include "nzBUS.h"
//#include "sizebits.h"


// monitor sizes in cells:
#define ZP_SIZEX 56 // 16 + Adress + borders
#define ZP_SIZEY 18

#define STK_SIZEX 56
#define STK_SIZEY 18

#define REG_SIZEX 30
#define REG_SIZEY 8


//special key codes:
#define KEY_G 113
#define KEY_RETURN 10

void initTerm(WINDOW * *monitor0, WINDOW * *monitorSTK, WINDOW * *monitorREG) {

    initscr();
    cbreak();
    noecho();

    start_color();

    init_pair(10, COLOR_RED, COLOR_BLACK);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_WHITE, COLOR_BLACK);

    //resize_term(61, 20);

    int xOffset, yOffset;
    xOffset = 1;
    yOffset = 1;

    *monitor0 = newwin(ZP_SIZEY, ZP_SIZEX, yOffset, xOffset);
    *monitorSTK = newwin(STK_SIZEY, STK_SIZEX, ZP_SIZEY + yOffset, xOffset);
    *monitorREG = newwin(REG_SIZEY, REG_SIZEX, yOffset, ZP_SIZEX + xOffset * 2);


    wattron(*monitor0, COLOR_PAIR(3));
    box(*monitor0, 0, 0);
    wattroff(*monitor0, COLOR_PAIR(3));

    wattron(*monitor0, COLOR_PAIR(2));
    mvwprintw(*monitor0, 0, 1, "ZERO PAGE:(%d, %d)", getmaxx(*monitor0), getmaxy(*monitor0));
    wattroff(*monitor0, COLOR_PAIR(2));

    wattron(*monitorSTK, COLOR_PAIR(3));
    box(*monitorSTK, 0, 0);
    wattroff(*monitorSTK, COLOR_PAIR(3));

    wattron(*monitorSTK, COLOR_PAIR(2));
    mvwprintw(*monitorSTK, 0, 1, "STACK:(%d, %d)", getmaxx(*monitorSTK), getmaxy(*monitorSTK));
    wattroff(*monitorSTK, COLOR_PAIR(2));

    wattron(*monitorREG, COLOR_PAIR(3));
    box(*monitorREG, 0, 0);
    wattroff(*monitorREG, COLOR_PAIR(3));

    wattron(*monitorREG, COLOR_PAIR(2));
    mvwprintw(*monitorREG, 0, 1, "REGISTERS:(%d, %d)", getmaxx(*monitorREG), getmaxy(*monitorREG));
    wattroff(*monitorREG, COLOR_PAIR(2));

    wattron(*monitor0, COLOR_PAIR(4));
    wattron(*monitorSTK, COLOR_PAIR(4));
    int section = 0;
    for (int pY = 1; pY < ZP_SIZEY - 1; pY ++){
        mvwprintw(*monitor0, pY, 1, "$%.4x: ", section);
        mvwprintw(*monitorSTK, pY, 1, "$%.4x: ", (0x0100 | section));
        section += 0x10;

    }
    wattroff(*monitor0, COLOR_PAIR(4));
    wattroff(*monitorSTK, COLOR_PAIR(4));

    wattron(*monitor0, COLOR_PAIR(5));
    wattron(*monitorSTK, COLOR_PAIR(5));
}

void update(WINDOW * *monitor0, WINDOW * *monitorSTK, WINDOW * *monitorREG, CPU * aCpu) {
   int pX, pY;

    pX = pY = 1;
    for (int i = 0; i <= 0xFF; i++){
        if (pX == 1) {
            pX += 7;
        }

        mvwprintw(*monitor0, pY, pX,"%.2x", cpuRead(i, false));
        mvwprintw(*monitorSTK, pY, pX,"%.2x", cpuRead(0x0100 | i, false));

        if (pX < ZP_SIZEX - 3){
            pX += 3;
        } else {
                pX = 1;
                pY++;
            }
        }

    mvwprintw(*monitorREG, 1, 1, "STATUS: ");

    Byte flags[] = {aCpu->flag_C, aCpu->flag_Z,
        aCpu->flag_I, aCpu->flag_D, aCpu->flag_B,
        aCpu->flag_U, aCpu->flag_V, aCpu->flag_N};

    for (int i = 0; i < 8; i++) {
        if (cpuIsActiveFlag(flags[i])) {
            wattron(*monitorREG, COLOR_PAIR(1));
        }
        if (flags[i] == aCpu->flag_C){
            mvwprintw(*monitorREG, 1, 9,  "C");
        }
        else if (flags[i] == aCpu->flag_Z){
            mvwprintw(*monitorREG, 1, 11, "Z");
        }
        else if (flags[i] == aCpu->flag_I){
            mvwprintw(*monitorREG, 1, 13, "I");
        }
        else if (flags[i] == aCpu->flag_D){
            mvwprintw(*monitorREG, 1, 15, "D");
        }
        else if (flags[i] == aCpu->flag_B){
            mvwprintw(*monitorREG, 1, 17, "B");
        }
        else if (flags[i] == aCpu->flag_U){
            mvwprintw(*monitorREG, 1, 19, "~");
        }
        else if (flags[i] == aCpu->flag_V){
            mvwprintw(*monitorREG, 1, 21, "V");
        }
        else if (flags[i] == aCpu->flag_N){
            mvwprintw(*monitorREG, 1, 23, "N");
        }
        wattroff(*monitorREG, COLOR_PAIR(1));
    }

    mvwprintw(*monitorREG, 2, 1, "PC:     $%.4x [%d]", aCpu->PgCount, aCpu->PgCount);
    mvwprintw(*monitorREG, 3, 1, "A:      $%.2x   [%d]", aCpu->A, aCpu->A);
    mvwprintw(*monitorREG, 4, 1, "X:      $%.2x   [%d]", aCpu->X, aCpu->X);
    mvwprintw(*monitorREG, 5, 1, "Y:      $%.2x   [%d]", aCpu->Y, aCpu->Y);
    mvwprintw(*monitorREG, 6, 1, "StkPtr: $%.2x   [%d]", aCpu->StkPtr, aCpu->StkPtr);

    refresh();
    wrefresh(*monitor0);
    wrefresh(*monitorSTK);
    wrefresh(*monitorREG);
}

int main(int argc, char *argv[])
{

    CPU newCpu;
    BUS newBus;

    cpuCreate(&newCpu);
    cpuConBus(&newBus);

    WINDOW * ZPpage;
    WINDOW * STKpage;
    WINDOW * REGmon;

    initTerm(&ZPpage, &STKpage, &REGmon);

    update(&ZPpage, &STKpage, &REGmon, &newCpu);
    cpuReset();
    cpuWrite(0x0ef, 0xAA);
    cpuWrite(0x0b1, 0xBB);
    cpuWrite(0x00eF, 0xCC);
    cpuWrite(0x0000, 0xAA);
    cpuWrite(0x01ff, 0xCC);
    cpuSetStaFlag(newCpu.flag_I, true);

    update(&ZPpage, &STKpage, &REGmon, &newCpu);

    int counter = 0;
    while (1) {

        int ch = getch();
        if (ch == KEY_G) {
            break;
        }

        else if (ch == KEY_RETURN) {
            counter++;
            cpuWrite(0x0000, counter);
            cpuSetStaFlag(newCpu.flag_I, false);
            update(&ZPpage, &STKpage, &REGmon, &newCpu);
        }

    }

    endwin();
    return 0;
}
