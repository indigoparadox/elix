
# vim: ft=make noexpandtab

BIN_WIN=qdwx.exe
BIN_LIN=qdlx
BIN_MSP430=qd430.bin
BIN_X86=qd86.bin
ISO_X86=qd86.iso
IMG_X86=qd86.img

define SYSLINUXCFG
DEFAULT $(BIN_X86)
LABEL $(BIN_X86)
   SAY Booting $(BIN_X86)
   KERNEL /$(BIN_X86)
endef

MD=mkdir -v -p
MKRESCUE=grub-mkrescue
DD=dd
MKVFAT=/sbin/mkfs.vfat
SYSLINUX=syslinux
MCOPY=mcopy

CFLAGS=-Wall -Werror -DUSE_CONSOLE -DUSE_NET -DUSE_DISK -I../../chiipy/src -DALPHA_PRESENT
SOURCES_TESTED= \
	net/tcp.c \
	net/ether.c \
	net/arp.c \
	mem.c \
	console.c \
	adhd.c \
	strings.c \
	alpha.c \
	commands.c \
	ring.c \
	io.c \
	mfat.c \
	mbmp.c
SOURCES=$(SOURCES_TESTED) \
	net/net.c

CHECKOBJ=\
	check.o \
	check_mem.o \
	check_ring.o \
	check_mfat.o \
	check_ether.o \
	check_net.o
#	check_console.o \
#	check_alpha.o \

OBJECTS=$(subst .c,.o,$(SOURCES))
OBJECTS_TESTED=$(subst .c,.o,$(SOURCES_TESTED))
LDFLAGS=

DRIVERS_TESTS= \
	colinux/display.o \
	colinux/keyboard.o \
	colinux/uart.o \
	colinux/disk.o

DRIVERS_COLINUX= \
	colinux/net.o \
	colinux/display.o \
	colinux/keyboard.o \
	colinux/uart.o \
	colinux/disk.o

DRIVERS_MSP430=\
	msp430/display.o \
	msp430/boot.o \
	msp430/net.o \
	msp430/uart.o \
	msp430/keyboard.o \
	msp430/wdt.o \
	msp430/disk.o

ISODIRX86=../isodir/x86
CHECKSDIR=../tests

$(BIN_LIN): CFLAGS += \
	-g \
	-DDEBUG \
	-DUSE_EXT_CLI \
	-DMPRINT \
	-DQD_PLATFORM=QD_PLATFORM_COLINUX \
	-DCOLINUX_TERMIOS \
	-DCOMMANDS_NET
$(BIN_LIN): LDFLAGS += -lpcap -lncurses -L../../chiipy/src/ -lchiipy
$(BIN_LIN): OBJDIR := ../obj/linux

$(BIN_WIN): CC := i686-w64-mingw32-g
$(BIN_WIN): CFLAGS += \
	-DNET_CON_ECHO \
	-g \
	-DQD_PLATFORM=QD_PLATFORM_COLINUX \
	-DCOMMANDS_NET
$(BIN_WIN): LDFLAGS += -lwpcap
$(BIN_WIN): OBJDIR := ../obj/windows

$(BIN_X86): CFLAGS += \
	-ffreestanding \
	-nostdlib \
	-nostartfiles \
	-nodefaultlibs \
	-DCOMMANDS_NET \
	-DQD_PLATFORM=QD_PLATFORM_X86
$(BIN_X86): OBJDIR := ../obj/os
$(BIN_X86): LDFLAGS := 

$(BIN_MSP430): CFLAGS += \
	-g3 \
	-O0 \
	-ggdb \
	-gdwarf-2 \
	-DQD_PLATFORM=QD_PLATFORM_MSP430 \
	-mmcu=msp430g2553 \
	-DQD_CONSOLE_IN=uart \
	-DQD_CONSOLE_OUT=uart \
	-DQD_CONSOLE_IN_IDX=1 \
	-DQD_CONSOLE_OUT_IDX=1 \
	-DCOMMANDS_NET \
	-DQD_UART_HW \
	-DQD_CPU_MHZ=1 \
	-DQD_SPI \
	-DQD_UART \
	-DQD_WDT_DISABLED
$(BIN_MSP430): CC := msp430-gcc
$(BIN_MSP430): STRIP := msp430-strip
$(BIN_MSP430): OBJDIR := ../obj/msp430
#$(BIN_MSP430): LDFLAGS += -L../../chiipy/src/ -lchiipy

tests: LDFLAGS += -lpcap $(shell pkg-config --libs check)
tests: CFLAGS += -DCHECK -g -DQD_PLATFORM=QD_PLATFORM_COLINUX -pg
tests: OBJDIR := ../obj/tests

all:

$(BIN_LIN): $(OBJECTS) $(DRIVERS_COLINUX) colinux/boot.o kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

$(BIN_WIN): $(OBJECTS) $(DRIVERS_COLINUX) kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

$(ISO_X86): $(ISODIRX86)/$(BIN_X86)
	$(MKRESCUE) -o $@ $(ISODIRX86)

$(IMG_X86): $(BIN_X86) syslinux.cfg
	$(DD) if=/dev/urandom of=$@ bs=512 count=2880
	$(MKVFAT) $@
	$(SYSLINUX) -i $@
	$(MCOPY) -i $@ $< ::

syslinux.cfg:
	echo "$$SYSLINUXCFG" >> $@

$(ISODIRX86)/$(BIN_X86): $(BIN_X86)
	$(MD) $(ISODIRX86)/boot
	cp -v $(BIN_X86) $(ISODIRX86)/boot/$(BIN_X86)

$(BIN_X86): $(OBJECTS) x86/display.o x86/net.o x86/keyboard.o kernel.o
	$(AS) x86/boot.s -o $(OBJDIR)/boot.o
	$(CC) -T x86/linker.ld -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS) $(CFLAGS)

$(BIN_MSP430): $(OBJECTS) $(DRIVERS_MSP430) kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(CFLAGS) $(LDFLAGS)

tests: $(OBJECTS_TESTED) $(DRIVERS_TESTS) $(addprefix $(CHECKSDIR)/,$(CHECKOBJ))
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$(subst $(CHECKSDIR),,$^)) $(LDFLAGS)

%.o: %.c
	$(MD) $(OBJDIR)/$(dir $@)
	$(CC) -c -o $(OBJDIR)/$@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf ../obj; \
	rm tests; \
	rm $(BIN_LIN); \
	rm $(BIN_WIN); \
	rm *.bin; \
	rm $(ISODIRX86)/boot/*.bin

