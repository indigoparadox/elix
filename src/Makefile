
MD=mkdir

CFLAGS=-Wall -g -DNET_CON_ECHO
SOURCES=./net/tcp.c ./net/ether.c ./vm.c ./net/arp.c ./mem.c
OBJECTS=$(subst .c,.o,$(SOURCES))
LDFLAGS=

lalixnet: CFLAGS += -DNET_CON_ECHO -g
lalixnet: LDFLAGS += -lpcap
lalixnet: OBJDIR := ../obj/linux

walixnet.exe: CC := i686-w64-mingw32-gcc
walixnet.exe: CFLAGS += -DNET_CON_ECHO -g
walixnet.exe: LDFLAGS += -lwpcap
walixnet.exe: OBJDIR := ../obj/windows

alix.bin: CFLAGS += -ffreestanding -DSILENT
alix.bin: OBJDIR := ../obj/os
alix.bin: LDFLAGS := -Ttext 0x0 --oformat binary

tests: LDFLAGS += -lcheck -lpthread -lm -lrt -lpcap
tests: OBJDIR := ../obj/tests

all:

lalixnet: $(OBJECTS) main.o drivers/vnet.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

walixnet.exe: $(OBJECTS) main.o drivers/vnet.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

alix.bin: $(OBJECTS)
	$(LD) -o $@ $(addprefix $(OBJDIR)/,$^) $(LDFLAGS)

tests: $(OBJECTS) drivers/vnet.o ../tests/check.o ../tests/check_ether.o
	$(CC) -o $@ $(addprefix $(OBJDIR)/,$(subst ../tests,,$^)) $(LDFLAGS)

%.o: %.c
	$(MD) -p $(OBJDIR)/$(dir $@)
	$(CC) -c -o $(OBJDIR)/$@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf ../obj ; rm tests ; rm lalixnet ; rm walixnet.exe

