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
unsigned char gfx[W * H];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short sp;
unsigned char draw_flag = 1;
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
unsigned char keymap[16] = {
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_4,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_W,
    SDL_SCANCODE_E,
    SDL_SCANCODE_R,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_F,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_X,
    SDL_SCANCODE_C,
    SDL_SCANCODE_V
};


int main(int argc, char *argv[])
{
    cpu_reset(argv[1]);

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

    /* SDL_Event event; */
    SDL_Renderer *renderer;
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    uint32_t ms = 15;

    for (;;) {
        emulate_cycle();

        if (draw_flag) {
            draw(renderer, ms);
        }

    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void draw_sprite(SDL_Renderer *renderer, SDL_Rect r)
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &r);
}


void draw(SDL_Renderer *renderer, uint32_t ms)
{
    SDL_Rect r;

    /* clear window */
    SDL_RenderClear(renderer);

    for (int i = 0; i < SCREEN_H; i++) {
        for (int j = 0; j < SCREEN_W; j++){
            if (gfx[(j/10) + (i/10) * 64] >= 1) {
                r.y = i;
                r.x = j;
                r.w = 1;
                r.h = 1;
                draw_sprite(renderer, r);
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_RenderPresent(renderer);

    SDL_Delay(ms);
}


void cpu_reset(char *game)
{
    I = 0;
    pc = 0x200;
    memset(V, 0, sizeof(V));
    FILE *in;
    in = fopen(game, "rb");
    fread(&memory[0x200], 0xFFF, 1, in);
    fclose(in);
    memset(gfx, 0, sizeof(gfx));
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
    /* printf("Instruction %X\n", opcode); */

    switch(opcode & 0xF000) {
    case 0xF000:
        switch(opcode & 0x00FF) {
        case 0x0007: /* 0xFX07 Sets VX to the value of the delay timer */
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            break;
        case 0x000A: /* 0xFX0A A key press is awaited, and then stored in VX */

            const uint8_t *keys = SDL_GetKeyboardState(NULL);

            for (int i = 0; i < 16; i++) {
                if (keys[keymap[i]]) {
                    V[(opcode & 0x0F00) >> 8] = i;
                    pc += 2;
                }
            }

            break;
        case 0x0015: /* 0xFX15 Sets the delay timer to VX */
            delay_timer = V[(opcode & 0x0F00) >> 8];
            break;
        case 0x0018: /* 0xFX18 Sets the sound timer to VX */
            sound_timer = V[(opcode & 0x0F00) >> 8];
            break;
        case 0x001E: /* 0xFX1E Adds VX to I */
            I += V[(opcode & 0x0F00) >> 8];
            break;
        case 0x0029: /* 0xFX29 Sets I to the location of the sprite for the
                      * character in VX. Characters 0-F (in hex) are represented
                      * by a 4x5 font.
                      */
            I = V[(opcode & 0x0F00) >> 8] * 5;
            break;
        case 0x0033: /* 0xFX33 Stores the Binary-coded decimal representation of
                      * VX, with the most significant of the three digits at the
                      * address in I, the middle digit at I plus 1, and the
                      * least significant digit at I plus 2.
                      */
            {
                unsigned char vx = V[(opcode * 0x0F00) >> 8];
                memory[I] = vx / 100;
                memory[I + 1] = (vx / 10) % 10;
                memory[I + 2] = vx % 10;
            }
            break;
        case 0x0055: /* 0xFX55 Stores V0 to VX in memory starting at address I */
            {
                int vx = (opcode & 0x0F00) >> 8;
                int v = 0;

                while (v <= vx) {
                    memory[I + v] = V[v];
                }

                I = I + vx + 1;
            }
            break;
        case 0x0060: /* 0xFX65 Fills V0 to VX with values from memory starting at
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
            }
            break;
        }
        break;
    case 0x0000:
        switch(opcode & 0x000F) {
        case 0x0000: /* 0x00E0: Clears the screen */
            memset(gfx, 0, sizeof(gfx));
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
            pc += 2;
        }
        break;
    case 0x4000: /* 0x4XNN Skips the next instruction if VX doesn't equal NN */
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
            pc += 2;
        }
        break;
    case 0x5000: /* 0x5XY0 Skips the next instruction if VX equals VY */
        if (V[(opcode & 0x0F00) >> 8] == (V[(opcode & 0x00F0) >> 4])) {
            pc += 2;
        }
        break;
    case 0x6000: /* 0x6XNN Sets VX to NN */
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
        break;
    case 0x7000: /* 0x7XNN Adds NN to VX */
        V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
        break;
    case 0x8000:
        switch(opcode & 0x000F) {
        case 0x0000: /* 0x8XY0 Sets VX to the value of VY */
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            break;
        case 0x0001: /* 0x8XY1 Sets VX to VX or VY */
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            break;
        case 0x0002: /* 0x8XY2 Sets VX to VX and VY */
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            break;
        case 0x0003: /* 0x8XY3 Sets VX to VX xor VY */
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            break;
        case 0x0004: /* 0x8XY4 Adds VY to VX. VF is set to 1 when there's a
                      * carry, and to 0 when there isn't.
                      */
            V[0xF] = 0;

            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                V[0xF] = 1; /* carry */
            }

            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            break;
        case 0x0005: /* 0x8XY5 VY is subtracted from VX. VF is set to 0 when
                      * there's a borrow, and to 1 when there isn't.
                      */
            V[0xF] = 1;

            if (V[opcode & 0x0F00 >> 8] < V[(opcode & 0x00F0) >> 4]) {
                V[0xF] = 0;
            }

            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            break;
        case 0x0006: /* 0x8XY6 Shifts VX right by one. VF is set to the value of
                      * the least significant bit of VX before the shift.
                      */
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            break;
        case 0x0007: /* 0x8XY7 Sets VX to VY minus VX. VF is set to 0 when
                      * there's a borrow, and 1 when there isn't.
                      */
            V[0xF] = 1;

            if (V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8]) {
                V[0xF] = 0;
            }

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            break;
        case 0x000E: /* 0x8XYE Shifts VX left by one. VF is set to the value of
                      * the most significant bit of VX before the shift.
                      */
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            break;
        }
        break;
    case 0x9000: /* 0x9XY0 Skips the next instruction if VX doesn't equal VY */
        if (V[(opcode & 0x0F000) >> 8] != V[(opcode & 0x00F0) >> 4]) {
            pc += 2;
        }
        break;
    case 0xA000: /* 0xANNN Sets I to the address NNN */
        I = opcode & 0x0FFF;
        break;
    case 0xB000: /* 0xBNNN Jumps to the address NNN plus V0 */
        pc = V[0] + (opcode & 0x0FFF);
        break;
    case 0xC000: /* 0xCXNN Sets VX to a random number, masked by NN */
        V[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
        break;
    case 0xD000: /* 0xDXYN Sprites stored in memory at location in index
                  * register (I), maximum 8bits wide. Wraps around the
                  * screen. If when drawn, clears a pixel, register VF is set to
                  * 1 otherwise it is zero. All drawing is XOR drawing (i.e. it
                  * toggles the screen pixels).
                  */
        {
            unsigned short vx = V[(opcode & 0x0F00) >> 8];
            unsigned short vy = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            V[0xF] &= 0;

            for (int y = 0; y < height; y++) {
                unsigned char pixel = memory[I + y];
                for (int x = 0; x < 8; x++) {
                    if (pixel & (0x80 >> x)) {
                        if (gfx[x+vx+(y+vy)*64]) {
                            V[0xF] = 1;
                        }
                        gfx[x+vx+(y+vy)*64] ^= 1;
                    }
                }
            }
        }

        draw_flag = 1;
        break;
    case 0xE000:
        switch(opcode & 0x00FF) {
        case 0x009E: /* 0xEX9E Skips the next instruction if the key stored in
                      * VX is pressed. */
            const uint8_t *keys = SDL_GetKeyboardState(NULL);
            if (keys[keymap[V[(opcode & 0x0F00) >> 8]]]) {
                pc += 4;
            }
            else {
                pc += 2;
            }
            break;
        case 0x00A1: /* 0xEXA1 Skips the next instruction if the key stored in
                      * VX isn't pressed. */
            const uint8_t *keys = SDL_GetKeyboardState(NULL);
            if (!keys[keymap[V[(opcode & 0x0F00) >> 8]]]) {
                pc += 4;
            }
            else {
                pc += 2;
            }
            break;
        }
        break;
    default:
        printf("%X: No match\n", opcode);
    }
}
