OUT = eulerian
CC = gcc
CSTD = -ansi
GCCW = -Wall -Werror -Wextra -Wunused -Wstrict-prototypes \
	-pedantic -pedantic-errors
CFLAGS = -m64 -O3 $(CSTD) $(GCCW)
LDFLAGS = 
VERSION = 0.2.2
objects = $(OUT).o

all: $(OUT)

$(OUT): $(objects)
	$(CC) -o $@ $(LDFLAGS) $^

$(OUT).o: $(OUT).c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(OUT) *.o *~ core.*

pack:
	tar cJf eulerian_$(VERSION)_src.txz eulerian.c Makefile LICENSE README

