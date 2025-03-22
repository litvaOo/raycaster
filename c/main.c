#include "player.h"
#include "upng.h"
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
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "defs.h"
#include "graphics.h"
#include "map.h"
#include "ray.h"

upng_t *textures[8];

void render_3D_projections(Uint32 *color_buffer, Ray *rays, Player *player) {
  for (int i = 0; i < NUM_RAYS; i++) {
    float distance =
        rays[i].distance * cos(rays[i].rayAngle - player->rotationAngle);
    float distance_to_projection_plane =
        (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
    float wall_strip_height =
        (TILE_SIZE / distance) * distance_to_projection_plane;

    float shade = wall_strip_height / WINDOW_HEIGHT;
    int y_start = WINDOW_HEIGHT / 2 - wall_strip_height / 2;
    if (y_start < 0)
      y_start = 0;
    int y_end = y_start + wall_strip_height;
    if (y_end >= WINDOW_HEIGHT)
      y_end = WINDOW_HEIGHT - 1;
    int texture_width = upng_get_width(textures[rays[i].wallHitContent - 1]);
    int texture_height = upng_get_height(textures[rays[i].wallHitContent - 1]);
    int texture_offset_x = rays[i].wasHitVertical
                               ? (int)(rays[i].wallHitY) % (int)TILE_SIZE
                               : (int)(rays[i].wallHitX) % (int)TILE_SIZE;

    for (int x = i * WALL_STRIP_WIDTH;
         x < i * WALL_STRIP_WIDTH + WALL_STRIP_WIDTH; x++) {
      for (int j = 0; j < y_start; j++) {
        color_buffer[j * (int)WINDOW_WIDTH + x] = 0xFFA9A9A9;
      }
      for (int y = y_start; y < y_end; y++) {
        int texture_offset_y =
            (y + (wall_strip_height / 2 - WINDOW_HEIGHT / 2)) *
            ((float)texture_height / wall_strip_height);
        uint32_t texel = ((uint32_t *)upng_get_buffer(
            textures[rays[i].wallHitContent -
                     1]))[texture_width * texture_offset_y + texture_offset_x];
        color_buffer[y * (int)WINDOW_WIDTH + x] =
            texel + ((int)(0xFF000000 * shade) & (0xFF000000));
      }
      for (int j = y_end; j < WINDOW_HEIGHT; j++) {
        color_buffer[j * (int)WINDOW_WIDTH + x] = 0xFF2F4F4F;
      }
    }
  }
}

void render(SDL_Renderer *renderer, SDL_Texture *texture, Uint32 *color_buffer,
            Player *player, Ray *rays) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  render_color_buffer(renderer, texture, color_buffer);
  clear_color_buffer(color_buffer, 0xFF00EE30);

  render_3D_projections(color_buffer, rays, player);
  render_map(color_buffer);
  render_rays(color_buffer, 0xFFFF0000, rays, player);
  SDL_RenderPresent(renderer);
}

void update(Player *player, Ray *rays) {
  player->rotationAngle += player->turnDirection * player->turnSpeed;
  float move_step = player->walkDirection * player->walkSpeed;

  float new_x = player->x + move_step * cos(player->rotationAngle);
  float new_y = player->y + move_step * sin(player->rotationAngle);
  if (map_content((int)(new_y / TILE_SIZE), (int)(new_x / TILE_SIZE)) == 0) {
    player->x = new_x;
    player->y = new_y;
  }
  cast_all_rays(player, rays);
}

int main(void) {
  SDL_Window *window = initializeWindow();
  SDL_Renderer *renderer = initializeRenderer(window);
  Player player = {
      WINDOW_WIDTH / 2,
      WINDOW_HEIGHT / 2,
      TILE_SIZE,
      TILE_SIZE,
      0,
      0,
      M_PI / 5,
      1,
      1 * (M_PI / 180),
  };

  Ray *rays = mmap(NULL, sizeof(Ray) * NUM_RAYS, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
  Uint32 *color_buffer =
      mmap(NULL, sizeof(Uint32) * (Uint32)WINDOW_WIDTH * (Uint32)WINDOW_HEIGHT,
           PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

  textures[0] = upng_new_from_file("c/images/redbrick.png");
  textures[1] = upng_new_from_file("c/images/purplestone.png");
  textures[2] = upng_new_from_file("c/images/mossystone.png");
  textures[3] = upng_new_from_file("c/images/graystone.png");
  textures[4] = upng_new_from_file("c/images/colorstone.png");
  textures[5] = upng_new_from_file("c/images/bluestone.png");
  textures[6] = upng_new_from_file("c/images/wood.png");
  textures[7] = upng_new_from_file("c/images/eagle.png");

  for (int i = 0; i < NUM_TEXTURES; i++) {
    assert(textures[i] != NULL);
    upng_decode(textures[i]);
    assert(upng_get_error(textures[i]) == UPNG_EOK);
  }

  SDL_Texture *color_buffer_texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
      WINDOW_WIDTH, WINDOW_HEIGHT);

  unsigned int last_frame_ticks = 0;
  while (true) {
    float delta_time = (SDL_GetTicks() - last_frame_ticks) / 1000.0;
    if (delta_time < 1000.0 / FRAME_RATE) {
      SDL_Delay(1000.0 / FRAME_RATE - delta_time);
    }
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
    case SDL_EVENT_KEY_UP:
      switch (event.key.scancode) {
      case SDL_SCANCODE_UP:
      case SDL_SCANCODE_DOWN:
        player.walkDirection = 0;
        break;
      case SDL_SCANCODE_LEFT:
      case SDL_SCANCODE_RIGHT:
        player.turnDirection = 0;
      default:
        break;
      }
      break;
    case SDL_EVENT_KEY_DOWN:
      if (event.key.scancode != SDL_SCANCODE_ESCAPE) {
        switch (event.key.scancode) {
        case SDL_SCANCODE_UP:
          player.walkDirection = 1;
          break;
        case SDL_SCANCODE_DOWN:
          player.walkDirection = -1;
          break;
        case SDL_SCANCODE_LEFT:
          player.turnDirection = -1;
          break;
        case SDL_SCANCODE_RIGHT:
          player.turnDirection = 1;
        default:
          break;
        }
        break;
      }
    case SDL_EVENT_QUIT:
      munmap(color_buffer,
             sizeof(Uint32) * (Uint32)WINDOW_WIDTH * (Uint32)WINDOW_HEIGHT);
      munmap(rays, sizeof(Ray) * NUM_RAYS);
      SDL_DestroyTexture(color_buffer_texture);
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return 0;
    }
    update(&player, rays);
    render(renderer, color_buffer_texture, color_buffer, &player, rays);
  }
}
