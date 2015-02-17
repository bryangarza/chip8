#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include "chip8.h"


#define SCREEN_W 640
#define SCREEN_H 320
#define W 64
#define H 32

unsigned short opcode;
unsigned char memory[4096];
unsigned char V[16];
unsigned short I;
unsigned short pc;
unsigned char gfx[64][32];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short sp;
unsigned char key[16];
unsigned char draw_flag;
unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


int main(int argc, char *argv[])
{
    cpu_reset();

    /* load fontset into memory */
    for(int i = 0; i < 80; i++) {
        memory[i] = chip8_fontset[i];
    }

    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Chip8",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_W,
        SCREEN_H,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer *renderer;
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    uint32_t ms = 15;
    /* gfx[3][4] = 1; */
    /* draw(renderer, ms); */
    /* gfx[5][10] = 1; */
    /* draw(renderer, ms); */

    for (;;) {
        emulate_cycle();
        draw(renderer, ms);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void draw(SDL_Renderer *renderer, uint32_t ms)
{

    SDL_Rect r;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    /* Clear window */
    SDL_RenderClear(renderer);

    for (int y = 0; y < W; y++) {
        for (int x = 0; x < H; x++) {
            if (gfx[y][x] != 0) {
                printf("Will draw");
                r.x = x;
                r.y = y;
                r.w = 10;
                r.h = 10;
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

    SDL_RenderFillRect(renderer, &r);

    SDL_RenderPresent(renderer);

    SDL_Delay(ms);
}


void cpu_reset()
{
    I = 0;
    pc = 0x200;
    memset(V, 0, sizeof(V));
    FILE *in;
    in = fopen("./c8games/Breakout.ch8", "rb");
    fread(&memory[0x200], 0xFFF, 1, in);
    /* for(int i = 0; i < (0xfff - 0x200); i++) { */
    /*     printf("%x ", memory[i+0x200]); */
    /* } */

    /* printf("%x", get_next_opcode()); */
    fclose(in);
}

unsigned short get_next_opcode()
{
    unsigned short opcode;
    opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;
    return opcode;
}

void emulate_cycle()
{
    opcode = get_next_opcode();
    printf("Instruction %X\n", opcode);

    switch(opcode & 0xF000) {
    case 0x0000:
        switch(opcode & 0x000F) {
        case 0x0000: /* 0x00E0: Clears the screen */
            memset(gfx, 0, sizeof(gfx));
            pc += 2;
            break;
        case 0x000E: /* 0x00EE: Returns from a subroutine */
            pc = stack[sp];
            --sp;
            break;
        }
        break;
    case 0x1000: /* 0x1NNN Jumps to address NNN */
        pc = opcode & 0x0FFF;
        break;
    case 0x2000: /* 0x2NNN Calls subroutine at NNN */
        stack[sp] = pc;
        ++sp;
        pc = opcode & 0x0FFF;
        break;
    case 0x3000: /* 0x3XNN Skips the next instruction if VX equals NN */
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
            pc += 4;
        }
        else {
            pc += 2;
        }
        break;
    case 0x4000: /* 0x4XNN Skips the next instruction if VX doesn't equal NN */
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
            pc += 4;
        }
        else {
            pc += 2;
        }
        break;
    case 0x5000: /* 0x5XY0 Skips the next instruction if VX equals VY */
        if (V[(opcode & 0x0F00) >> 8] == (V[(opcode & 0x00F0) >> 4])) {
            pc += 4;
        }
        else {
            pc += 2;
        }
        break;
    case 0x6000: /* 0x6XNN Sets VX to NN */
        V[(opcode & 0x0F00) >> 8] = (opcode * 0x00FF);
        pc += 2;
        break;
    case 0x7000: /* 0x7XNN Adds NN to VX */
        V[(opcode & 0x0F00) >> 8] += (opcode * 0x00FF);
        pc += 2;
        break;
    case 0x8000:
        switch(opcode & 0x000F) {
        case 0x0000: /* 0x8XY0 Sets VX to the value of VY */
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0001: /* 0x8XY1 Sets VX to VX or VY */
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0002: /* 0x8XY2 Sets VX to VX and VY */
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0003: /* 0x8XY3 Sets VX to VX xor VY */
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0004: /* 0x8XY4 Adds VY to VX. VF is set to 1 when there's a
                      * carry, and to 0 when there isn't.
                      */
            V[0xF] = 0;

            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                V[0xF] = 1; /* carry */
            }

            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0005: /* 0x8XY5 VY is subtracted from VX. VF is set to 0 when
                      * there's a borrow, and to 1 when there isn't.
                      */
            V[0xF] = 1;

            if (V[opcode & 0x0F00 >> 8] < V[(opcode & 0x00F0) >> 4]) {
                V[0xF] = 0;
            }

            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0006: /* 0x8XY6 Shifts VX right by one. VF is set to the value of
                      * the least significant bit of VX before the shift.
                      */
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;
        case 0x0007: /* 0x8XY7 Sets VX to VY minus VX. VF is set to 0 when
                      * there's a borrow, and 1 when there isn't.
                      */
            V[0xF] = 1;

            if (V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8]) {
                V[0xF] = 0;
            }

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x000E: /* 0x8XYE Shifts VX left by one. VF is set to the value of
                      * the most significant bit of VX before the shift.
                      */
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;
        }
        break;
    case 0x9000: /* 0x9XY0 Skips the next instruction if VX doesn't equal VY */
        if (V[(opcode & 0x0F000) >> 8] != V[(opcode & 0x00F0) >> 4]) {
            pc += 4;
        }
        else {
            pc += 2;
        }
        break;
    case 0xA000: /* 0xANNN Sets I to the address NNN */
        I = opcode & 0x0FFF;
        pc += 2;
        break;
    case 0xB000: /* 0xBNNN Jumps to the address NNN plus V0 */
        pc = V[0] + (opcode & 0x0FFF);
        break;
    case 0xC000: /* 0xCXNN Sets VX to a random number, masked by NN */
        V[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
        pc += 2;
        break;
    case 0xD000: /* 0xDXYN Sprites stored in memory at location in index
                  * register (I), maximum 8bits wide. Wraps around the
                  * screen. If when drawn, clears a pixel, register VF is set to
                  * 1 otherwise it is zero. All drawing is XOR drawing (i.e. it
                  * toggles the screen pixels).
                  */
        printf(" opcode %X, ie 0xDXYN, XOR drawing.....\n", opcode);
        {
            unsigned short coordx = V[(opcode & 0x0F00) >> 8];
            unsigned short coordy = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;

            V[0xF] = 0;

            for (int yline = 0; yline < height; yline++) {
                unsigned short data = memory[I + yline];
                unsigned char xpixelinv = 7;

                for (int xpixel = 0; xpixel < 8; xpixel++, xpixelinv--) {
                    unsigned char mask = 1 << xpixelinv;
                    if (data & mask) {

                        unsigned char x = coordx + xpixel;
                        unsigned char y = coordy + yline;

                        if (gfx[x][y] == 1) {
                            /* Collision */
                            V[0xF] = 1;
                        }
                        gfx[x][y] ^= 1;
                    }
                }
            }

            /* for (int yline = 0; yline < height; yline++) { */
            /*     pixel = memory[I + yline]; */
            /*     for (int xline = 0; xline < 8; xline++) { */
            /*         if ((pixel & (0x80 >> xline)) != 0) { */
            /*             if (gfx[(x + xline + ((y + yline) * 64))] == 1) { */
            /*                 V[0xF] = 1; */
            /*             } */
            /*             gfx[x + xline + ((y + yline) * 64)] ^= 1; */
            /*         } */
            /*     } */
            /* } */
            draw_flag = 1;
            pc += 2;
        }
        break;
    case 0xE000:
        switch(opcode & 0x00FF) {
        case 0x009E: /* 0xEX9E Skips the next instruction if the key stored in
                      * VX is pressed.
                      */
            if (key[V[(opcode & 0x0F00) >> 8]] == 1) {
                pc += 4;
            }
            else {
                pc += 2;
            }
            break;
        case 0x00A1: /* 0xEXA1 Skips the next instruction if the key stored in
                      * VX isn't pressed.
                      */
            if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
                pc += 4;
            }
            else {
                pc += 2;
            }
            break;
        }
        break;
    case 0xF000:
        switch(opcode & 0x00F0) {
        case 0x0000:
            switch(opcode & 0x000F) {
            case 0x0007: /* 0xFX07 Sets VX to the value of the delay timer */
                V[(opcode & 0x0F00) >> 8] = delay_timer;
                pc += 2;
                break;
            case 0x000A: /* 0xFX0A A key press is awaited, and then stored in
                          * VX
                          */
                /* TODO: fill this in, needs SDL instruction as well */
                break;
            }
            break;
        case 0x0010:
            switch(opcode & 0x000F) {
            case 0x0005: /* 0xFX15 Sets the delay timer to VX */
                delay_timer = V[(opcode & 0x0F00) >> 8];
                pc += 2;
                break;
            case 0x0008: /* 0xFX18 Sets the sound timer to VX */
                sound_timer = V[(opcode & 0x0F00) >> 8];
                pc += 2;
                break;
            case 0x000E: /* 0xFX1E Adds VX to I */
                I += V[(opcode & 0x0F00) >> 8];
                pc += 2;
                break;
            }
            break;
        case 0x0020: /* 0xFX29 Sets I to the location of the sprite for the
                      * character in VX. Characters 0-F (in hex) are represented
                      * by a 4x5 font.
                      */
            I = V[(opcode & 0x0F00) >> 8] * 5;
            break;
        case 0x0030: /* 0xFX33 Stores the Binary-coded decimal representation of
                      * VX, with the most significant of the three digits at the
                      * address in I, the middle digit at I plus 1, and the
                      * least significant digit at I plus 2.
                      */
            {
                unsigned char vx = V[(opcode * 0x0F00) >> 8];
                memory[I] = vx / 100;
                memory[I + 1] = (vx / 10) % 10;
                memory[I + 2] = vx % 10;
                pc += 2;
            }
            break;
        case 0x0050: /* 0xFX55 Stores V0 to VX in memory starting at address I */
            {
                int vx = (opcode & 0x0F00) >> 8;
                int v = 0;

                while (v <= vx) {
                    memory[I + v] = V[v];
                }

                I = I + vx + 1;
                pc += 2;
            }
            break;
        case 0x0060: /* Fills V0 to VX with values from memory starting at
                      * address I
                      */
            {
                int vx = (opcode & 0x0F00) >> 8;
                int v = 0;

                while (v <= vx) {
                    V[v] = memory[I + v];
                    v++;
                }

                I = I + vx + 1;
                pc += 2;
            }
            break;
        }
        default:
            printf("%X: No match\n", opcode);
            break;
        break;
    }
}
