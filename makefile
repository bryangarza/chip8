cc = /usr/local/bin/x86_64-apple-darwin14.1.0-gcc-4.9.3

build:
	$(cc) -Wall -std=c99 -o chip8 chip8.c `sdl2-config --cflags --libs`

run:
	make build
	./chip8 ./c8games/breakout.ch8

debug:
	$(cc) -g -Wall -std=c99 -o chip8 chip8.c `sdl2-config --cflags --libs`

clean:
	-rm chip8
	-rm -r chip8.dSYM
