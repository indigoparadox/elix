
# vim: ft=make noexpandtab

BIN_WIN=qdwx.exe
BIN_LIN=qdlx
BIN_LIN_MIN=qdlx-min
BIN_MSP430=qd430.bin
BIN_X86=qd86.bin
ISO_X86=qd86.iso
IMG_X86=qd86.img

define SYSLINUXCFG =
DEFAULT $(BIN_X86)
LABEL $(BIN_X86)
   SAY Booting $(BIN_X86)
   KERNEL /$(BIN_X86)
endef

SYSLINUX_MODS=/usr/lib/syslinux/modules/bios

MD=mkdir -v -p
MKRESCUE=grub-mkrescue
DD=dd
MKVFAT=/sbin/mkfs.vfat
SYSLINUX=syslinux
MCOPY=mcopy

DEBUG_FLAGS=-Wall -Werror -g
PROFILE_FLAGS=$(DEBUG_FLAGS) -pg

CFLAGS=\
	-DUSE_CONSOLE \
	-DUSE_DISK \
	-I../../chiipy/src \
	-I../../mfat/src \
	-I../../mbmp/src \
	-DVERSION=\"$(shell date +'%Y.%m.%d.%H.%M.%S')\"

SOURCES= \
	console.c \
	adhd.c \
	alpha.c \
	mem.c

SOURCES_APPS_COLINUX= \
	apps/repl.c \
	apps/microd.c

SOURCES_APPS_X86=$(SOURCES_APPS_COLINUX)

SOURCES_APPS_MSP430=\
	apps/repl.c
#	apps/xmodem.c

SOURCES_TESTED_NET= \
	net/tcp.c \
	net/ether.c \
	net/arp.c
SOURCES_NET=$(SOURCES_TESTED_NET) \
	net/net.c

CHECKOBJ=\
	check.o \
	check_mem.o \
	check_ether.o \
	check_net.o
#	check_console.o \
#	check_alpha.o \

OBJECTS=$(subst .c,.o,$(SOURCES))
OBJECTS_APPS_X86=$(subst .c,.o,$(SOURCES_APPS_X86))
OBJECTS_APPS_COLINUX=$(subst .c,.o,$(SOURCES_APPS_COLINUX))
OBJECTS_APPS_MSP430=$(subst .c,.o,$(SOURCES_APPS_MSP430))
OBJECTS_NET=$(subst .c,.o,$(SOURCES_NET))
OBJECTS_TESTED_NET=$(subst .c,.o,$(SOURCES_TESTED_NET))
LDFLAGS=

DRIVERS_TESTS= \
	colinux/display.o \
	colinux/keyboard.o \
	colinux/disk.o

DRIVERS_COLINUX= \
	colinux/net.o \
	colinux/display.o \
	colinux/keyboard.o \
	colinux/disk.o

DRIVERS_MSP430=\
	msp430/boot.o \
	msp430/uart.o \
	msp430/wdt.o \
	msp430/disk.o \
	msp430/irqal.o

DRIVERS_X86=\
	x86/display.o \
	x86/net.o \
	x86/keyboard.o \
	x86/disk.o \

ISODIRX86=../isodir/x86
CHECKSDIR=../tests

$(BIN_LIN): CFLAGS += \
	-DUSE_EXT_CLI \
	-DUSE_NET \
	-DUSE_BMP \
	-DINIT_TASK="trepl_task" \
	-DQD_PLATFORM=QD_PLATFORM_COLINUX \
	-DDEBUG \
	-DMBMP_AGGRESSIVE_VALIDATE \
	-DCOLINUX_TERMIOS \
	$(DEBUG_FLAGS)
#	-DALPHA_CMP_DEBUG
$(BIN_LIN): LDFLAGS += \
	-lpcap \
	-lncurses \
	-L../../mfat/bin/linux \
	-lmfat \
	-L../../mbmp/bin/linux \
	-lmbmp
#	-L../../chiipy/bin/linux/ \
#	-lchiipy
$(BIN_LIN): OBJDIR := ../obj/linux/

$(BIN_WIN): CC := i686-w64-mingw32-g
$(BIN_WIN): CFLAGS += \
	-DNET_CON_ECHO \
	-DUSE_EXT_CLI \
	-DUSE_NET \
	-DQD_PLATFORM=QD_PLATFORM_COLINUX \
	-DCOMMANDS_NET
$(BIN_WIN): LDFLAGS += -lwpcap
$(BIN_WIN): OBJDIR := ../obj/windows/

$(BIN_X86): CC := ia16-elf-gcc
$(BIN_X86): AS := ia16-elf-as
$(BIN_X86): CFLAGS += \
	-Os \
	-ffreestanding \
	-nostdlib \
	-DQD_PLATFORM=QD_PLATFORM_X86 \
	-DINIT_TASK="trepl_task" \
	-DGCC_REALMODE \
	-nodefaultlibs \
	-nostartfiles
$(BIN_X86): OBJDIR := ../obj/os/
$(BIN_X86): LDFLAGS := -T x86/linker.ld -lgcc

# Try to match the MSP430 target as closely as possible in test env.
$(BIN_LIN_MIN): CFLAGS += \
	-g3 \
	-O0 \
	-ggdb \
	-gdwarf-2 \
	-DDEBUG \
	-DCOLINUX_TERMIOS \
	-DMEM_HEAP_SIZE=320 \
	-DINIT_TASK="uartping_task" \
	-DQD_PLATFORM=QD_PLATFORM_COLINUX \
	-DUSE_DISK_RO
$(BIN_LIN_MIN): OBJDIR := ../obj/colinux-min/
$(BIN_LIN_MIN): LDFLAGS += \
	-lpcap \
	-L../../mfat/bin/linux \
	-lmfat

$(BIN_MSP430): CFLAGS += \
	-g3 \
	-O0 \
	-ggdb \
	-gdwarf-2 \
	-Wall \
	-Werror \
	-DMEM_HEAP_SIZE=300 \
	-DUART_TX_BUFFER_LENGTH=0 \
	-DUART_RX_BUFFER_LENGTH=20 \
	-DINIT_TASK="trepl_task" \
	-DQD_PLATFORM=QD_PLATFORM_MSP430 \
	-mmcu=msp430g2553 \
	-DUTOA_BITS=32 \
	-DCONSOLE_IN="uart" \
	-DCONSOLE_OUT="uart" \
	-DUSE_DISK_RO \
	-DQD_UART_HW \
	-DQD_CPU_MHZ=16 \
	-DQD_SPI \
	-DQD_UART \
	-DQD_WDT_DISABLED
$(BIN_MSP430): CC := msp430-gcc
$(BIN_MSP430): OBJDIR := ../obj/msp430/
$(BIN_MSP430): LDFLAGS += \
	-L../../mfat/bin/msp430g2553 \
	-lmfat \
	-L../../msign/bin/msp430g2553 \
	-lmsign

tests: LDFLAGS += -lpcap $(shell pkg-config --libs check)
tests: CFLAGS += \
	-DCHECK \
	-DQD_PLATFORM=QD_PLATFORM_COLINUX \
	-DUSE_NET \
	-DMEM_SAVE
tests: OBJDIR := ../obj/tests/

all:

$(BIN_LIN): $(OBJECTS) $(OBJECTS_NET) $(OBJECTS_APPS_COLINUX) $(DRIVERS_COLINUX) colinux/boot.o kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR),$^) $(LDFLAGS)

$(BIN_LIN_MIN): $(OBJECTS) $(OBJECTS_NET) $(OBJECTS_APPS_MSP430) $(DRIVERS_COLINUX) colinux/boot.o kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR),$^) $(LDFLAGS)

$(BIN_WIN): $(OBJECTS) $(OBJECTS_NET) $(DRIVERS_COLINUX) kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR),$^) $(LDFLAGS)

$(ISO_X86): $(ISODIRX86)/$(BIN_X86)
	$(MKRESCUE) -o $@ $(ISODIRX86)

$(IMG_X86): $(BIN_X86) syslinux.cfg
	$(DD) if=/dev/urandom of=$@ bs=512 count=2880
	$(MKVFAT) $@
	$(SYSLINUX) -i $@
	$(MCOPY) -i $@ $^ $(SYSLINUX_MODS)/mboot.c32 $(SYSLINUX_MODS)/libcom32.c32 ::

$(ISODIRX86)/$(BIN_X86): $(BIN_X86)
	$(MD) $(ISODIRX86)/boot
	cp -v $(BIN_X86) $(ISODIRX86)/boot/$(BIN_X86)

$(BIN_X86): $(OBJECTS) $(OBJECTS_NET) $(OBJECTS_APPS_X86) $(DRIVERS_X86) kernel.o
	$(AS) x86/boot.s -o $(OBJDIR)boot.o
	$(CC) -o $@ $(addprefix $(OBJDIR),$^) $(OBJDIR)boot.o $(LDFLAGS) $(CFLAGS)

$(BIN_MSP430): $(OBJECTS) $(DRIVERS_MSP430) $(OBJECTS_APPS_MSP430) kernel.o
	$(CC) -o $@ $(addprefix $(OBJDIR),$^) $(CFLAGS) $(LDFLAGS)

tests: $(OBJECTS_TESTED) $(OBJECTS_TESTED_NET) $(DRIVERS_TESTS) $(addprefix $(CHECKSDIR)/,$(CHECKOBJ))
	$(CC) -o $@ $(addprefix $(OBJDIR),$(subst $(CHECKSDIR),,$^)) $(LDFLAGS)

docs:
	doxygen doxyfile

%.o: %.c
	$(MD) $(OBJDIR)$(dir $@)
	$(CC) -c -o $(OBJDIR)$@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf ../obj; \
	rm tests; \
	rm $(BIN_LIN); \
	rm $(BIN_WIN); \
	rm $(IMG_X86); \
	rm $(ISO_X86); \
	rm $(BIN_LIN_MIN); \
	rm *.bin; \
	rm $(ISODIRX86)/boot/*.bin

