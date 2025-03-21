#include "graphics.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

SDL_Window *initializeWindow(void) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == false) {
    fprintf(stderr, "Error initializing SDL %s\n", SDL_GetError());
    exit(1);
  }
  SDL_Window *window = SDL_CreateWindow("Raycaster", WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
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
  for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
    color_buffer[i] = color;
}

void render_color_buffer(SDL_Renderer *renderer, SDL_Texture *texture,
                         Uint32 *color_buffer) {
  SDL_UpdateTexture(texture, NULL, color_buffer, WINDOW_WIDTH * sizeof(Uint32));
  SDL_RenderTexture(renderer, texture, NULL, NULL);
}

void draw_rectangle(Uint32 *color_buffer, Uint32 color, int x, int y,
                    float width, float height) {
  for (int i = x; i <= x + width; i++) {
    for (int j = y; j <= y + height; j++) {
      color_buffer[j * (int)WINDOW_WIDTH + i] = color;
    }
  }
}
