PREFIX  ?= /opt/amiga
CC      = $(PREFIX)/bin/m68k-amigaos-gcc
CFLAGS  = -noixemul -m68020 -O2 -Wall -Wno-pointer-sign -Iinclude -Iinclude/cnet
LDFLAGS = -noixemul
LIBS    = -Llib -lcnet_door

CNETC_DOORS = \
    Minesweeper \
	MinesweeperCGFX

ALL_EXAMPLES = $(CNETC_DOORS) 
ALL_TARGETS = $(ALL_EXAMPLES) 
.PHONY: all clean lib examples templates tools tests
all: lib examples templates tools
examples: lib $(ALL_EXAMPLES)

# CNetC doors need the door library
$(CNETC_DOORS): %: %.c lib
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	$(MAKE) -C lib PREFIX=$(PREFIX) clean
	rm -f $(ALL_TARGETS)
