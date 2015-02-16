#include <string.h>
#include <stdio.h>

#include "chip8.h"

unsigned short opcode;
unsigned char memory[4096];
unsigned char V[16];
unsigned short I;
unsigned short pc;
unsigned char gfx[64 * 32];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short sp;
unsigned char key[16];

int main(int argc, char *argv[])
{
    cpu_reset();
    return(0);
}

void cpu_reset()
{
    I = 0;
    pc = 0x200;
    memset(V, 0, sizeof(V));
    FILE *in;
    in = fopen("./c8games/PONG", "rb");
    fread(&memory[0x200], 0xfff, 1, in);
    /* for(int i = 0; i < (0xfff - 0x200); i++) { */
    /*     printf("%x ", memory[i+0x200]); */
    /* } */

    /* printf("%x", get_next_opcode()); */
    fclose(in);
}

short get_next_opcode()
{
    unsigned short opcode;
    opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;
    return opcode;
}
