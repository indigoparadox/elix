
# vim: ft=make noexpandtab

MD=mkdir -v -p
MKRESCUE=grub-mkrescue
DD=dd
MKVFAT=/sbin/mkfs.vfat
SYSLINUX=syslinux
MCOPY=mcopy

CFLAGS=-Wall
SOURCES=net/tcp.c net/ether.c net/arp.c mem.c console.c net/net.c adhd.c stdlib.c
CHECKOBJ=check.o check_ether.o check_mem.o check_console.o
OBJECTS=$(subst .c,.o,$(SOURCES))
LDFLAGS=

DRIVERS_COLINUX=colinux/net.o colinux/display.o colinux/keyboard.o

ISODIRX86=../isodir/x86
CHECKSDIR=../tests

lalixnet: CFLAGS += -g -DDEBUG -DMPRINT -DPLATFORM_COLINUX
lalixnet: LDFLAGS += -lpcap -lcurses
lalixnet: OBJDIR := ../obj/linux

walixnet.exe: CC := i686-w64-mingw32-gcc
walixnet.exe: CFLAGS += -DNET_CON_ECHO -g -DPLATFORM_COLINUX
walixnet.exe: LDFLAGS += -lwpcap
walixnet.exe: OBJDIR := ../obj/windows

alix86.bin: CFLAGS += -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -DPLATFORM_X86 -DCONSOLE_COLOR
alix86.bin: OBJDIR := ../obj/os
alix86.bin: LDFLAGS := 

alix430.bin: CFLAGS += -DPLATFORM_MSP430 -mmcu=msp430g2553 -DCONSOLE_SERIAL
alix430.bin: CC := msp430-gcc
alix430.bin: STRIP := msp430-strip
alix430.bin: OBJDIR := ../obj/msp430

tests: LDFLAGS += -lcheck -lpthread -lm -lrt -lpcap
tests: CFLAGS += -DCHECK
tests: OBJDIR := ../obj/tests

all:

lalixnet: $(OBJECTS) $(DRIVERS_COLINUX) colinux/boot.o kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

walixnet.exe: $(OBJECTS) $(DRIVERS_COLINUX) kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

alix86.iso: $(ISODIRX86)/alix86.bin
	$(MKRESCUE) -o $@ $(ISODIRX86)

alix86.img: alix86.bin
	$(DD) if=/dev/urandom of=$@ bs=512 count=2880
	$(MKVFAT) $@
	$(SYSLINUX) -i $@
	$(MCOPY) -i $@ ../syslinux.cfg ::
	$(MCOPY) -i $@ $< ::

$(ISODIRX86)/alix86.bin: alix86.bin
	$(MD) $(ISODIRX86)/boot
	cp -v alix86.bin $(ISODIRX86)/boot/alix86.bin

alix86.bin: $(OBJECTS) x86/display.o x86/net.o x86/keyboard.o kernel.o
	$(AS) x86/boot.s -o $(OBJDIR)/boot.o
	$(CC) -T x86/linker.ld -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS) $(CFLAGS)

alix430.bin: $(OBJECTS) msp430/display.o msp430/boot.o msp430/net.o kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(CFLAGS) $(LDFLAGS)

tests: $(OBJECTS) $(addprefix $(CHECKSDIR)/,$(CHECKOBJ))
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$(subst $(CHECKSDIR),,$^)) $(LDFLAGS)

%.o: %.c
	$(MD) $(OBJDIR)/$(dir $@)
	$(CC) -c -o $(OBJDIR)/$@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf ../obj; \
	rm tests; \
	rm lalixnet; \
	rm walixnet.exe; \
	rm *.bin; \
	rm $(ISODIRX86)/boot/*.bin

