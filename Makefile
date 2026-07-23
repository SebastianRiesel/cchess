CFLAGS+=-Wall -Wextra -g

all: sdl_main

sdl_main: SDL_main.c cchess cchess_ai cchess_tcp
	cc $(CFLAGS) SDL_main.c cchess.o cchess_ai.o cchess_tcp.o -o cchess  $(shell pkg-config --cflags --libs sdl3) $(shell pkg-config --cflags --libs sdl3-image)

main: cchess
	cc $(CFLAGS) main.c cchess.o -o main

cchess_ai: cchess
	cc $(CFLAGS) cchess.o -c cchess_ai.c

cchess_tcp: cchess
	cc $(CFLAGS) cchess.o -c cchess_tcp.c

cchess: cchess.c cchess.h
	cc $(CFLAGS) -c cchess.c





