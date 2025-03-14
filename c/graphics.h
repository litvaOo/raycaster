#pragma once

#include "defs.h"

SDL_Window *initializeWindow(void);
SDL_Renderer *initializeRenderer(SDL_Window *window);
void clear_color_buffer(Uint32 *color_buffer, Uint32 color);
void render_color_buffer(SDL_Renderer *renderer, SDL_Texture *texture, Uint32 *color_buffer);
