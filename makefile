cc = /usr/local/bin/gcc-4.9

build:
	$(cc) -Wall -std=c99 -o chip8 chip8.c `sdl2-config --cflags --libs`

run:
	make build
	./chip8

debug:
	$(cc) -g -Wall -std=c99 -o chip8 chip8.c `sdl2-config --cflags --libs`

clean:
	-rm chip8
	-rm -r chip8.dSYM
