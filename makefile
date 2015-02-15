test:
	make build
	make run

build:
	gcc -Wall -std=c99 -o chip8 chip8.c

run:
	./chip8

clean:
	-rm chip8
