
CFLAGS=-Wall -g -DNET_CON_ECHO
DEPS=tcp.o drivers/vnet.o bstrlib.o ether.o vm.o

all: alixnet tests

alixnet: main.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)

tests: $(DEPS) ../tests/check.c ../tests/check_ether.c
	$(CC) -o $@ $^ $(CFLAGS) -lcheck -lpthread -lm -lrt

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o ; rm -f drivers/*.o ; rm -f ../tests/*.o ; rm tests ; rm alixnet

