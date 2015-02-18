void cpu_reset();
unsigned short get_next_opcode();
void emulate_cycle();
void draw_sprite(SDL_Renderer *renderer, SDL_Rect r);
void draw(SDL_Renderer *renderer, uint32_t ms);
