
CFLAGS=-Wall -g -DNET_CON_ECHO
DEPS=net/tcp.o drivers/vnet.o bstrlib.o net/ether.o vm.o net/arp.o

all: alixnet tests

alixnet: main.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) -lpcap

tests: $(DEPS) ../tests/check.c ../tests/check_ether.c
	$(CC) -o $@ $^ $(CFLAGS) -lcheck -lpthread -lm -lrt -lpcap

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o ; rm -f drivers/*.o ; rm -f ../tests/*.o ; rm tests ; rm alixnet

