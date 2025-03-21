#pragma once

#include "defs.h"
#include "graphics.h"
#include <stdint.h>
#include <stdlib.h>

void render_map(Uint32 *color_buffer);
int map_content(int x, int y);
