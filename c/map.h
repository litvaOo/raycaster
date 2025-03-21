#pragma once
#include "defs.h"
#include "graphics.h"

void render_map(SDL_Renderer *renderer, Uint32 *color_buffer);
int map_content(int x, int y);
