#pragma once

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
