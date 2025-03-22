#pragma once

#include "defs.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

SDL_Window *initializeWindow(void);
SDL_Renderer *initializeRenderer(SDL_Window *window);
void clear_color_buffer(Uint32 *color_buffer, Uint32 color);
void render_color_buffer(SDL_Renderer *renderer, SDL_Texture *texture,
                         Uint32 *color_buffer);
void draw_rectangle(Uint32 *color_buffer, Uint32 color, int x, int y,
                    float width, float height);
void draw_line(int x0, int y0, int x1, int y1, Uint32 color,
               Uint32 *color_buffer);
