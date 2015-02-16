test:
	make build
	make run

build:
	gcc -Wall -std=c99 -o chip8 chip8.c `sdl2-config --cflags --libs`

run:
	./chip8

clean:
	-rm chip8
