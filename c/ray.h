#pragma once

typedef struct Ray Ray;

struct Ray {
  float rayAngle;
  float wallHitX;
  float wallHitY;
  float distance;
  int wallHitContent;
  bool wasHitVertical;
};
