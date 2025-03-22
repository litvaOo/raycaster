#include "ray.h"
#include "defs.h"
#include "graphics.h"
#include "player.h"

float normalizeAngle(float angle) {
  angle = remainder(angle, M_PI * 2);
  if (angle < 0) {
    angle = M_PI * 2 + angle;
  }
  return angle;
}

void cast_all_rays(Player *player, Ray *rays) {
  float projection_plane_distance = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
  for (int ray_id = 0; ray_id < NUM_RAYS; ray_id++) {
    float newRay =
        normalizeAngle(player->rotationAngle + atan((ray_id - NUM_RAYS / 2) /
                                                    projection_plane_distance));
    bool isRayDown = newRay > 0 && newRay < M_PI;
    bool isRayRight = newRay < 0.5 * M_PI || newRay > 1.5 * M_PI;
    bool hit_horizonal, hit_vertical = false;

    // horizontal interception
    float horizontal_y_intercept =
        floor(player->y / TILE_SIZE) * TILE_SIZE + (isRayDown ? TILE_SIZE : 0);
    float horizontal_x_intercept =
        player->x + (horizontal_y_intercept - player->y) / tan(newRay);

    float horizontal_x_step = TILE_SIZE / tan(newRay);
    horizontal_x_step *= (!isRayRight && horizontal_x_step > 0 ? -1 : 1);
    horizontal_x_step *= (isRayRight && horizontal_x_step < 0 ? -1 : 1);
    float horizontal_y_step = TILE_SIZE * (!isRayDown ? -1 : 1);

    float next_horizontal_touch_x = horizontal_x_intercept;
    float next_horizontal_touch_y = horizontal_y_intercept;

    float horizontal_wall_hit_x = 0, horizontal_wall_hit_y = 0;

    int horizontal_wall_id_x = 0, horizontal_wall_id_y = 0;

    while (next_horizontal_touch_x >= 0 &&
           next_horizontal_touch_x <= MAP_NUM_COLS * TILE_SIZE &&
           next_horizontal_touch_y >= 0 &&
           next_horizontal_touch_y <= MAP_NUM_ROWS * TILE_SIZE) {
      if (map_content(
              (int)floor((next_horizontal_touch_y + (!isRayDown ? -1 : 0)) /
                         TILE_SIZE),
              (int)floor(next_horizontal_touch_x / TILE_SIZE)) != 0) {
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
        floor(player->x / TILE_SIZE) * TILE_SIZE + (isRayRight ? TILE_SIZE : 0);
    float vertical_y_intercept =
        player->y + (vertical_x_intercept - player->x) * tan(newRay);

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
           (next_vertical_touch_x <= MAP_NUM_COLS * TILE_SIZE) &&
           (next_vertical_touch_y >= 0) &&
           (next_vertical_touch_y <= MAP_NUM_ROWS * TILE_SIZE)) {
      if (map_content((int)(next_vertical_touch_y / TILE_SIZE),
                      (int)((next_vertical_touch_x + (!isRayRight ? -1 : 0)) /
                            TILE_SIZE)) != 0) {
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
        hit_horizonal ? sqrt(pow(player->x - horizontal_wall_hit_x, 2) +
                             pow(player->y - horizontal_wall_hit_y, 2))
                      : FLT_MAX;
    float vertical_hit_distance =
        hit_vertical ? sqrt(pow(player->x - vertical_wall_hit_x, 2) +
                            pow(player->y - vertical_wall_hit_y, 2))
                     : FLT_MAX;

    float res_x, res_y, distance = 0;
    int wallHitContent = 0;
    bool end_hit_vertical = false;
    if (horizontal_hit_distance <= vertical_hit_distance) {
      res_x = horizontal_wall_hit_x;
      res_y = horizontal_wall_hit_y;
      distance = horizontal_hit_distance;
      wallHitContent = map_content(horizontal_wall_id_y, horizontal_wall_id_x);
    } else {
      res_x = vertical_wall_hit_x;
      res_y = vertical_wall_hit_y;
      distance = vertical_hit_distance;
      wallHitContent = map_content(vertical_wall_id_y, vertical_wall_id_x);
      end_hit_vertical = true;
    }

    rays[ray_id] =
        (Ray){newRay, res_x, res_y, distance, wallHitContent, end_hit_vertical};
  }
}

void render_rays(Uint32 *color_buffer, Uint32 color, Ray *rays,
                 Player *player) {
  for (int i = 0; i < NUM_RAYS; i++) {
    draw_line(player->x * MINIMAP_SCALE_FACTOR,
              player->y * MINIMAP_SCALE_FACTOR,
              rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
              rays[i].wallHitY * MINIMAP_SCALE_FACTOR, color, color_buffer);
  }
}
