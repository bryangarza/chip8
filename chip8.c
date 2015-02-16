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
    fread(&memory[0x200], 0xFFF, 1, in);
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

void emulate_cycle()
{
    opcode = get_next_opcode();

    switch(opcode * 0xF000) {
        case 0x0000:
            switch(opcode & 0x000F) {
                case 0x0000: // 0x00E0: Clears the screen
                    // Execute opcode
                    break;
                case 0x000E: // 0x00EE: Returns from a subroutine
                    // Execute opcode
                    break;
                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
        case 0x1000: // 0x1NNN: Jumps to address NNN
            break;
        case 0x2000: // 0x2NNN: Calls subroutine at NNN
            break;
        case 0x3000: // 0x3XNN Skips the next instruction if VX equals NN
            break;
    }
}
