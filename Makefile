CFLAGS+=-Wall -Wextra -ggdb3

all: main sdl_main

sdl_main: SDL_main.c cchess
	cc $(CFLAGS) SDL_main.c cchess.o -o SDL_main  $(shell pkg-config --cflags --libs sdl3) $(shell pkg-config --cflags --libs sdl3-image)

main: cchess
	cc $(CFLAGS) main.c cchess.o -o main
cchess: cchess.c cchess.h
	cc $(CFLAGS) -c cchess.c

