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
        case 0x0000: /* 0x00E0: Clears the screen */
            break;
        case 0x000E: /* 0x00EE: Returns from a subroutine */
            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
        break;
    case 0x1000: /* 0x1NNN Jumps to address NNN */
        break;
    case 0x2000: /* 0x2NNN Calls subroutine at NNN */
        break;
    case 0x3000: /* 0x3XNN Skips the next instruction if VX equals NN */
        break;
    case 0x4000: /* 0x4XNN Skips the next instruction if VX doesn't equal NN */
        break;
    case 0x5000: /* 0x5XY0 Skips the next instruction if VX equals VY */
        break;
    case 0x6000: /* 0x6XNN Sets VX to NN */
        break;
    case 0x7000: /* 0x7XNN Sets NN to VX */
        break;
    case 0x8000:
        switch(opcode & 0x000F) {
        case 0x0000: /* 0x8XY0 Sets VX to the value of VY */
            break;
        case 0x0001: /* 0x8XY1 Sets VX to VX or VY */
            break;
        case 0x0002: /* 0x8XY2 Sets VX to VX and VY */
            break;
        case 0x0003: /* 0x8XY3 Sets VX to VX xor VY */
            break;
        case 0x0004: /* 0x8XY4 Adds VY to VX. VF is set to 1 when there's a
                      * carry, and to 0 when there isn't.
                      */
            break;
        case 0x0005: /* 0x8XY5 VY is subtracted from VX. VF is set to 0 when
                      * there's a borrow, and to 1 when there isn't.
                      */
            break;
        case 0x0006: /* 0x8XY6 Shifts VX right by one. VF is set to the value of
                      * the least significant bit of VX before the shift.
                      */
            break;
        case 0x0007: /* 0x8XY7 Sets VX to VY minus VX. VF is set to 0 when
                      * there's a borrow, and 1 when there isn't.
                      */
            break;
        case 0x000E: /* 0x8XYE Shifts VX left by one. VF is set to the value of
                      * the most significant bit of VX before the shift.
                      */
            break;
        }
        break;
    case 0x9000: /* 0x9XY0 Skips the next instruction if VX doesn't equal VY */
        break;
    case 0xA000: /* 0xANNN Sets I to the address NNN */
        break;
    case 0xB000:
        break;
    case 0xC000:
        break;
    case 0xD000:
        break;
    case 0xE000:
        switch(opcode & 0x000F) {
        case 0x000E: /* 0xEX9E */
            break;
        case 0x0001: /* 0xEXA1 */
            break;
        }
        break;
    case 0xF000:
        switch(opcode & 0x00F0) {
        case 0x0000:
            switch(opcode & 0x000F) {
            case 0x0007: /* 0xFX07 */
                break;
            case 0x000A: /* 0xFX0A */
                break;
            }
            break;
        case 0x0010:
            switch(opcode & 0x000F) {
            case 0x0005: /* 0xFX15 Sets the delay timer to VX */
                break;
            case 0x0008: /* 0xFX18 Sets the sound timer to VX */
                break;
            case 0x000E: /* 0xFX1E Adds VX to I */
                break;
            }
            break;
        case 0x0020: /* 0xFX29 Sets I to the location of the sprite for the
                      * character in VX. Characters 0-F (in hex) are represented
                      * by a 4x5 font.
                      */
            break;
        case 0x0030: /* 0xFX33 Stores the Binary-coded decimal representation of
                      * VX, with the most significant of the three digits at the
                      * address in I, the middle digit at I plus 1, and the
                      * least significant digit at I plus 2.
                      */
            break;
        case 0x0050: /* 0xFX55 Stores V0 to VX in memory starting at address I */
            break;
        case 0x0060: /* Fills V0 to VX with values from memory starting at
                      * address I
                      */
            break;
        }
        break;
    }
}
