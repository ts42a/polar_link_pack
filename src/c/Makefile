CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
OBJ = pack.o varint.o crc16.o example.o

all: example

pack.o: pack.c pack.h
	$(CC) $(CFLAGS) -c pack.c -o $@

varint.o: varint.c varint.h
	$(CC) $(CFLAGS) -c varint.c -o $@

crc16.o: crc16.c crc16.h
	$(CC) $(CFLAGS) -c crc16.c -o $@

example.o: example.c pack.h varint.h crc16.h
	$(CC) $(CFLAGS) -c example.c -o $@

example: $(OBJ)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(OBJ) -o ../bin/example

clean:
	rm -f *.o
	rm -rf ../bin
