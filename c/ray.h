#pragma once

#include "defs.h"
#include "map.h"
#include "player.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
typedef struct Ray Ray;

struct Ray {
  float rayAngle;
  float wallHitX;
  float wallHitY;
  float distance;
  int wallHitContent;
  bool wasHitVertical;
};

float normalizeAngle(float angle);
void cast_all_rays(Player *player, Ray *rays);
void render_rays(Uint32 *color_buffer, Uint32 color, Ray *rays);
