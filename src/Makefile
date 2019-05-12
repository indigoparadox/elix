
CFLAGS=-Wall -g -DNET_CON_ECHO
DEPS=main.o tcp.o drivers/vnet.o bstrlib.o ether.o vm.o

all: alixnet

alixnet: $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o ; rm -f drivers/*.o ; rm alixnet

