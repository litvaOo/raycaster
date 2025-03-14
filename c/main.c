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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "defs.h"
#include "graphics.h"

typedef struct Player Player;

struct Player {
  float x;
  float y;
  float width;
  float height;
  float turnDirection;
  float walkDirection;
  float rotationAngle;
  float walkSpeed;
  float turnSpeed;
};

typedef struct Ray Ray;

struct Ray {
  float rayAngle;
  float wallHitX;
  float wallHitY;
  float distance;
  int wallHitContent;
  bool wasHitVertical;
  bool isRayFacingUp;
  bool isRayFacingDown;
  bool isRayFacingLeft;
  bool isRayFacingRight;
};

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

Ray *rays;

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 2, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5}};

upng_t *textures[8];

void render_3D_projections(Uint32 *color_buffer) {
  for (int i = 0; i < NUM_RAYS; i++) {
    float distance =
        rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
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

void render_map(SDL_Renderer *renderer) {
  for (int i = 0; i < MAP_NUM_ROWS; i++) {
    for (int j = 0; j < MAP_NUM_COLS; j++) {
      int tile_x = j * TILE_SIZE * MINIMAP_SCALE_FACTOR;
      int tile_y = i * TILE_SIZE * MINIMAP_SCALE_FACTOR;
      int tile_color = 255 * map[i][j];
      SDL_SetRenderDrawColor(renderer, tile_color, tile_color, tile_color, 255);
      SDL_FRect map_tile = {tile_x, tile_y, TILE_SIZE * MINIMAP_SCALE_FACTOR,
                            TILE_SIZE * MINIMAP_SCALE_FACTOR};
      SDL_RenderFillRect(renderer, &map_tile);
    }
  }
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  for (int i = 0; i < NUM_RAYS; i++) {
    SDL_RenderLine(renderer, player.x * MINIMAP_SCALE_FACTOR,
                   player.y * MINIMAP_SCALE_FACTOR,
                   rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
                   rays[i].wallHitY * MINIMAP_SCALE_FACTOR);
  }
}


void render(SDL_Renderer *renderer, SDL_Texture *texture, Uint32 *color_buffer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  render_color_buffer(renderer, texture, color_buffer);
  clear_color_buffer(color_buffer, 0xFF00EE30);

  render_map(renderer);
  render_3D_projections(color_buffer);
  SDL_RenderPresent(renderer);
}

float normalizeAngle(float angle) {
  angle = remainder(angle, M_PI * 2);
  if (angle < 0) {
    angle = M_PI * 2 + angle;
  }
  return angle;
}

void cast_all_rays(void) {
  float projection_plane_distance = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
  for (int ray_id = 0; ray_id < NUM_RAYS; ray_id++) {
    float newRay =
        normalizeAngle(player.rotationAngle + atan((ray_id - NUM_RAYS / 2) /
                                                   projection_plane_distance));
    bool isRayDown = newRay > 0 && newRay < M_PI;
    bool isRayRight = newRay < 0.5 * M_PI || newRay > 1.5 * M_PI;
    bool hit_horizonal, hit_vertical = false;

    // horizontal interception
    float horizontal_y_intercept =
        floor(player.y / TILE_SIZE) * TILE_SIZE + (isRayDown ? TILE_SIZE : 0);
    float horizontal_x_intercept =
        player.x + (horizontal_y_intercept - player.y) / tan(newRay);

    float horizontal_x_step = TILE_SIZE / tan(newRay);
    horizontal_x_step *= (!isRayRight && horizontal_x_step > 0 ? -1 : 1);
    horizontal_x_step *= (isRayRight && horizontal_x_step < 0 ? -1 : 1);
    float horizontal_y_step = TILE_SIZE * (!isRayDown ? -1 : 1);

    float next_horizontal_touch_x = horizontal_x_intercept;
    float next_horizontal_touch_y = horizontal_y_intercept;

    float horizontal_wall_hit_x = 0, horizontal_wall_hit_y = 0;

    int horizontal_wall_id_x = 0, horizontal_wall_id_y = 0;

    while (next_horizontal_touch_x >= 0 &&
           next_horizontal_touch_x <= MAP_NUM_COLS*TILE_SIZE &&
           next_horizontal_touch_y >= 0 &&
           next_horizontal_touch_y <= MAP_NUM_ROWS*TILE_SIZE) {
      if (map[(int)floor((next_horizontal_touch_y + (!isRayDown ? -1 : 0)) /
                         TILE_SIZE)]
             [(int)floor(next_horizontal_touch_x / TILE_SIZE)] != 0) {
        horizontal_wall_hit_x = next_horizontal_touch_x;
        horizontal_wall_hit_y = next_horizontal_touch_y;
        horizontal_wall_id_y =
            (int)((next_horizontal_touch_y - (!isRayDown ? 1 : 0)) / TILE_SIZE);
        horizontal_wall_id_x = (int)(next_horizontal_touch_x / TILE_SIZE);
        hit_horizonal = true;
        break;
      } else {
        next_horizontal_touch_x += horizontal_x_step;
        next_horizontal_touch_y += horizontal_y_step;
      }
    }

    // vertical_interception
    float vertical_x_intercept =
        floor(player.x / TILE_SIZE) * TILE_SIZE + (isRayRight ? TILE_SIZE : 0);
    float vertical_y_intercept =
        player.y + (vertical_x_intercept - player.x) * tan(newRay);

    float vertical_y_step = TILE_SIZE * tan(newRay);
    vertical_y_step *= (!isRayDown && vertical_y_step > 0 ? -1 : 1);
    vertical_y_step *= (isRayDown && vertical_y_step < 0 ? -1 : 1);
    float vertical_x_step = TILE_SIZE * (!isRayRight ? -1 : 1);

    float next_vertical_touch_x = vertical_x_intercept;
    float next_vertical_touch_y = vertical_y_intercept;

    float vertical_wall_hit_x = 0;
    float vertical_wall_hit_y = 0;
    int vertical_wall_id_x, vertical_wall_id_y = 0;

    while ((next_vertical_touch_x >= 0) &&
           (next_vertical_touch_x <= MAP_NUM_COLS*TILE_SIZE) &&
           (next_vertical_touch_y >= 0) &&
           (next_vertical_touch_y <= MAP_NUM_ROWS * TILE_SIZE)) {
      if (map[(int)(next_vertical_touch_y / TILE_SIZE)]
             [(int)((next_vertical_touch_x + (!isRayRight ? -1 : 0)) /
                    TILE_SIZE)] != 0) {
        vertical_wall_hit_x = next_vertical_touch_x;
        vertical_wall_hit_y = next_vertical_touch_y;
        vertical_wall_id_x =
            (int)((next_vertical_touch_x - (!isRayRight ? 1 : 0)) / TILE_SIZE);
        vertical_wall_id_y = (int)(next_vertical_touch_y / TILE_SIZE);
        hit_vertical = true;
        break;
      } else {
        next_vertical_touch_x += vertical_x_step;
        next_vertical_touch_y += vertical_y_step;
      }
    }

    float horizontal_hit_distance =
        hit_horizonal ? sqrt(pow(player.x - horizontal_wall_hit_x, 2) +
                             pow(player.y - horizontal_wall_hit_y, 2))
                      : FLT_MAX;
    float vertical_hit_distance =
        hit_vertical ? sqrt(pow(player.x - vertical_wall_hit_x, 2) +
                            pow(player.y - vertical_wall_hit_y, 2))
                     : FLT_MAX;

    float res_x, res_y, distance = 0;
    int wallHitContent = 0;
    bool end_hit_vertical = false;
    if (horizontal_hit_distance <= vertical_hit_distance) {
      res_x = horizontal_wall_hit_x;
      res_y = horizontal_wall_hit_y;
      distance = horizontal_hit_distance;
      wallHitContent = map[horizontal_wall_id_y][horizontal_wall_id_x];
    } else {
      res_x = vertical_wall_hit_x;
      res_y = vertical_wall_hit_y;
      distance = vertical_hit_distance;
      wallHitContent = map[vertical_wall_id_y][vertical_wall_id_x];
      end_hit_vertical = true;
    }

    rays[ray_id] = (Ray){
        newRay,           res_x,      res_y,     distance,    wallHitContent,
        end_hit_vertical, !isRayDown, isRayDown, !isRayRight, isRayRight,
    };
  }
}

void update(void) {
  player.rotationAngle += player.turnDirection * player.turnSpeed;
  float move_step = player.walkDirection * player.walkSpeed;

  float new_x = player.x + move_step * cos(player.rotationAngle);
  float new_y = player.y + move_step * sin(player.rotationAngle);
  if (map[(int)(new_y / TILE_SIZE)][(int)(new_x / TILE_SIZE)] == 0) {
    player.x = new_x;
    player.y = new_y;
  }
  cast_all_rays();
}

int main(void) {
  SDL_Window *window = initializeWindow();
  SDL_Renderer *renderer = initializeRenderer(window);
  rays = mmap(NULL, sizeof(Ray) * NUM_RAYS, PROT_READ | PROT_WRITE,
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
    update();
    render(renderer, color_buffer_texture, color_buffer);
  }
}
