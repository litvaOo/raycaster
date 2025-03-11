#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define FRAME_RATE 60

#define PI 3.14159265
#define TWO_PI 6.28318530
#define TILE_SIZE 64.0
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20
#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)
#define WALL_STRIP_WIDTH 1

#define MINIMAP_SCALE_FACTOR 0.3

#define FOV_ANGLE (60 * (PI / 180))

#define NUM_RAYS WINDOW_WIDTH

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
  float wallHitContent;
  bool wasHitVertical;
  bool isRayFacingUp;
  bool isRayFacingDown;
  bool isRayFacingLeft;
  bool isRayFacingRight;
};

Player player = {
    WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, TILE_SIZE, TILE_SIZE, 0, 0, PI / 2, 1,
    1 * (PI / 180),
};

Ray *rays;

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

SDL_Window *initializeWindow() {
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
  for (int i = 0; i < NUM_RAYS; i++) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderLine(renderer, player.x * MINIMAP_SCALE_FACTOR,
                   player.y * MINIMAP_SCALE_FACTOR,
                   rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
                   rays[i].wallHitY * MINIMAP_SCALE_FACTOR);
  }
}

void render_player(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_FRect player_rect = {player.x * MINIMAP_SCALE_FACTOR,
                           player.y * MINIMAP_SCALE_FACTOR,
                           player.width * MINIMAP_SCALE_FACTOR,
                           player.height * MINIMAP_SCALE_FACTOR};
  SDL_RenderFillRect(renderer, &player_rect);
  SDL_RenderLine(
      renderer, player.x * MINIMAP_SCALE_FACTOR,
      player.y * MINIMAP_SCALE_FACTOR,
      player.x * MINIMAP_SCALE_FACTOR + cos(player.rotationAngle) * 40,
      player.y * MINIMAP_SCALE_FACTOR + sin(player.rotationAngle) * 40);
}

void render_walls(SDL_Renderer *renderer) {
  for (int i = 0; i < NUM_RAYS; i++) {
    float distance =
        rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
    float distance_to_projection_plane =
        (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
    float wall_strip_height =
        (TILE_SIZE / distance) * distance_to_projection_plane;

    float shade = wall_strip_height / WINDOW_HEIGHT;
    SDL_SetRenderColorScale(renderer, 1 * shade);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer,
                       &(SDL_FRect){i * WALL_STRIP_WIDTH,
                                    (WINDOW_HEIGHT / 2 - wall_strip_height / 2),
                                    WALL_STRIP_WIDTH, wall_strip_height});
  }
  SDL_SetRenderColorScale(renderer, 1);
}

void render(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  render_walls(renderer);
  render_map(renderer);
  render_player(renderer);
  SDL_RenderPresent(renderer);
}

float normalizeAngle(float angle) {
  angle = fmod(angle, TWO_PI);
  if (angle < 0) {
    angle = TWO_PI + angle;
  }
  return angle;
}

void cast_all_rays(void) {
  float ray_angle = player.rotationAngle - (FOV_ANGLE / 2);
  for (int ray_id = 0; ray_id < NUM_RAYS; ray_id++) {
    float newRay = normalizeAngle(ray_angle);
    bool isRayDown = newRay > 0 && newRay < PI;
    bool isRayRight = newRay < 0.5 * PI || newRay > 1.5 * PI;

    // horizontal interception
    float horizontal_y_intercept =
        ((int)(player.y / TILE_SIZE)) * TILE_SIZE + (isRayDown ? TILE_SIZE : 0);
    float horizontal_x_intercept =
        player.x + (horizontal_y_intercept - player.y) / tan(newRay);

    float horizontal_x_step = TILE_SIZE / tan(newRay);
    horizontal_x_step *= (!isRayRight && horizontal_x_step > 0 ? -1 : 1);
    horizontal_x_step *= (isRayRight && horizontal_x_step < 0 ? -1 : 1);
    float horizontal_y_step = TILE_SIZE * (!isRayDown ? -1 : 1);

    float next_horizontal_touch_x = horizontal_x_intercept;
    float next_horizontal_touch_y = horizontal_y_intercept;

    float horizontal_wall_hit_x, horizontal_wall_hit_y;

    while (next_horizontal_touch_x >= 0 &&
           next_horizontal_touch_x <= WINDOW_WIDTH &&
           next_horizontal_touch_y >= 0 &&
           next_horizontal_touch_y <= WINDOW_HEIGHT) {
      if (map[(int)((next_horizontal_touch_y - (!isRayDown ? 1 : 0)) /
                    TILE_SIZE)][(int)(next_horizontal_touch_x / TILE_SIZE)] ==
          1) {
        horizontal_wall_hit_x = next_horizontal_touch_x;
        horizontal_wall_hit_y = next_horizontal_touch_y;
        break;
      } else {
        next_horizontal_touch_x += horizontal_x_step;
        next_horizontal_touch_y += horizontal_y_step;
      }
    }

    // vertical_interception
    float vertical_x_intercept =
        (int)(player.x / TILE_SIZE) * TILE_SIZE + (isRayRight ? TILE_SIZE : 0);
    float vertical_y_intercept =
        player.y + (vertical_x_intercept - player.x) * tan(newRay);

    float vertical_y_step = TILE_SIZE * tan(newRay);
    vertical_y_step *= (!isRayDown && vertical_y_step > 0 ? -1 : 1);
    vertical_y_step *= (isRayDown && vertical_y_step < 0 ? -1 : 1);
    float vertical_x_step = TILE_SIZE * (!isRayRight ? -1 : 1);

    float next_vertical_touch_x = vertical_x_intercept;
    float next_vertical_touch_y = vertical_y_intercept;

    float vertical_wall_hit_x;
    float vertical_wall_hit_y;

    while ((next_vertical_touch_x >= 0) &&
           (next_vertical_touch_x <= WINDOW_WIDTH) &&
           (next_vertical_touch_y >= 0) &&
           (next_vertical_touch_y <= WINDOW_HEIGHT)) {
      if (map[(int)(next_vertical_touch_y / TILE_SIZE)]
             [(int)((next_vertical_touch_x - (!isRayRight ? 1 : 0)) /
                    TILE_SIZE)] == 1) {
        vertical_wall_hit_x = next_vertical_touch_x;
        vertical_wall_hit_y = next_vertical_touch_y;
        break;
      } else {
        next_vertical_touch_x += vertical_x_step;
        next_vertical_touch_y += vertical_y_step;
      }
    }

    float horizontal_hit_distance =
        sqrt(pow(player.x - horizontal_wall_hit_x, 2) +
             pow(player.y - horizontal_wall_hit_y, 2));
    float vertical_hit_distance = sqrt(pow(player.x - vertical_wall_hit_x, 2) +
                                       pow(player.y - vertical_wall_hit_y, 2));

    float res_x, res_y, distance;

    bool hit_vertical = false;
    if (horizontal_hit_distance < vertical_hit_distance) {
      res_x = horizontal_wall_hit_x;
      res_y = horizontal_wall_hit_y;
      distance = horizontal_hit_distance;
    } else {
      res_x = vertical_wall_hit_x;
      res_y = vertical_wall_hit_y;
      distance = vertical_hit_distance;
      hit_vertical = true;
    }

    rays[ray_id] = (Ray){
        newRay,       res_x,      res_y,     distance,    0,
        hit_vertical, !isRayDown, isRayDown, !isRayRight, isRayRight,
    };
    ray_angle += FOV_ANGLE / NUM_RAYS;
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
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return 0;
    }
    update();
    render(renderer);
  }
}
