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
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL_video.h>
#include "graphics.h"

SDL_Window *initializeWindow(void) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == false) {
    fprintf(stderr, "Error initializing SDL %s\n", SDL_GetError());
    exit(1);
  }
  const SDL_DisplayMode *display_mode = SDL_GetCurrentDisplayMode(1);
  int true_width = display_mode->w;
  int true_height = display_mode->h;
  SDL_Window *window = SDL_CreateWindow("Raycaster", true_width,
                                        true_height, SDL_WINDOW_BORDERLESS);
  if (window == NULL) {
    fprintf(stderr, "Error initializing SDL window %s\n", SDL_GetError());
    exit(1);
  }
  return window;
}

SDL_Renderer *initializeRenderer(SDL_Window *window) {
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    fprintf(stderr, "Error initializing SDL renderer %s\n", SDL_GetError());
    exit(1);
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  return renderer;
}

void clear_color_buffer(Uint32 *color_buffer, Uint32 color) {
  for (int i = 0; i < WINDOW_WIDTH*WINDOW_HEIGHT; i++)
      color_buffer[i] = color;
}

void render_color_buffer(SDL_Renderer *renderer, SDL_Texture *texture, Uint32 *color_buffer) {
  SDL_UpdateTexture(texture, NULL, color_buffer, WINDOW_WIDTH * sizeof(Uint32));
  SDL_RenderTexture(renderer, texture, NULL, NULL);
}

