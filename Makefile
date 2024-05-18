
COMP = gcc
OUT = emuDebug
FLAGS = -c -Wall
PKGS = -lncurses


all: nz6502.o nzBUS.o
	$(COMP) -Wall nz6502.o nzBUS.o debugerFrame.c $(PKGS) -o $(OUT)

nz6502.o: nz6502.c
	$(COMP) $(FLAGS) nz6502.c

nzBUS.o: nzBUS.c
	$(COMP) $(FLAGS) nzBUS.c

exec:
	./emuDebug

#clean:
#	rm -rf emuDebug
#	rm -rf *.o
