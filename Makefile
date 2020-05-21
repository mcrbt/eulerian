NAME = eulerian
VERSION = 0.2.2
CC = gcc
CSTD = -ansi
ARCH = 64
WARN = -Wall -Werror -Wextra -Wunused -Wstrict-prototypes \
	-pedantic -pedantic-errors
CFLAGS = -m$(ARCH) -O3 $(CSTD) $(WARN)

all: $(NAME)

$(NAME): $(NAME).c
	$(CC) -o $@ $(CFLAGS) $<

clean:
	rm -f $(NAME) *.o *~ core.*

pack:
	tar czf eulerian_$(VERSION).tgz $(NAME).c Makefile LICENSE README.md
