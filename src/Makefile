
MD=mkdir

CFLAGS=-Wall
SOURCES=net/tcp.c net/ether.c net/arp.c mem.c console.c kernel.c
OBJECTS=$(subst .c,.o,$(SOURCES))
LDFLAGS=

lalixnet: CFLAGS += -DNET_CON_ECHO -g
lalixnet: LDFLAGS += -lpcap
lalixnet: OBJDIR := ../obj/linux

walixnet.exe: CC := i686-w64-mingw32-gcc
walixnet.exe: CFLAGS += -DNET_CON_ECHO -g
walixnet.exe: LDFLAGS += -lwpcap
walixnet.exe: OBJDIR := ../obj/windows

alix86.bin: CFLAGS += -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -DPLATFORM_X86 -DCONSOLE_COLOR
alix86.bin: OBJDIR := ../obj/os
alix86.bin: LDFLAGS := -Ttext 0x0 --oformat binary

alix430.bin: CFLAGS += -DPLATFORM_MSP430 -mmcu=msp430g2553 -DCONSOLE_SERIAL
alix430.bin: CC := msp430-gcc
alix430.bin: STRIP := msp430-strip
alix430.bin: OBJDIR := ../obj/msp430

tests: LDFLAGS += -lcheck -lpthread -lm -lrt -lpcap
tests: OBJDIR := ../obj/tests

all:

lalixnet: $(OBJECTS) colinux/net.o colinux/display.o colinux/boot.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

walixnet.exe: $(OBJECTS) main.o drivers/vnet.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

alix86.bin: $(OBJECTS) x86/display.o x86/net.o
	$(LD) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

alix430.bin: $(OBJECTS) msp430/display.o msp430 msp430/boot.o msp430/net.o
	$(LD) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

tests: $(OBJECTS) drivers/vnet.o ../tests/check.o ../tests/check_ether.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$(subst ../tests,,$^)) $(LDFLAGS)

%.o: %.c
	$(MD) -p $(OBJDIR)/$(dir $@)
	$(CC) -c -o $(OBJDIR)/$@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf ../obj ; rm tests ; rm lalixnet ; rm walixnet.exe ; rm *.bin

