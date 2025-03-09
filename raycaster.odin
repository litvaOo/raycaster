package raycaster

import "vendor:sdl3"
import "core:fmt" 
import "core:strings"
import "core:math"


Player::struct {
  x: f64, 
  y: f64,
  radius: f64,
  turnDirection: f64, // -1 if left, +1 if right
  walkDirection: f64, // -1 if back, +1 if right
  rotationAngle: f64,
  moveSpeed: f64,
  rotationSpeed: f64
}


TILE_SIZE :: 64
MAP_NUM_ROWS :: 11
MAP_NUM_COLS :: 15
FOV_ANGLE :: 60 * (math.PI/180)
WALL_STRIP_WIDTH :: 1
NUM_RAYS :: WINDOW_WIDTH / WALL_STRIP_WIDTH
WINDOW_WIDTH :: MAP_NUM_COLS*TILE_SIZE 
WINDOW_HEIGHT :: MAP_NUM_ROWS*TILE_SIZE 

MINIMAP_SCALE_FACTOR :: 0.2

walls := [MAP_NUM_ROWS*MAP_NUM_COLS]u8{
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
  1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
}

render_circle::proc(renderer: ^sdl3.Renderer, center_x: f64, center_y: f64, radius: int) {
  arrSize := i32(((radius * 8 * 35 / 49) + (8 - 1)) & -8)
  points := make([^]sdl3.FPoint, arrSize)
  drawCount := 0
  diameter := radius * 2

  x := f64(radius - 1)
  y := f64(0)

  tx := 1
  ty := 1

  error := tx - diameter

  for x >= y {
    points[drawCount+0] = {f32( center_x + x ), f32(center_y - y)}
    points[drawCount+1] = {f32( center_x + x ), f32(center_y + y)}
    points[drawCount+2] = {f32( center_x - x ), f32(center_y - y)}
    points[drawCount+3] = {f32( center_x - x ), f32(center_y + y)}
    points[drawCount+4] = {f32( center_x + y ), f32(center_y - x)}
    points[drawCount+5] = {f32( center_x + y ), f32(center_y + x)}
    points[drawCount+6] = {f32( center_x - y ), f32(center_y - x)}
    points[drawCount+7] = {f32( center_x - y ), f32(center_y + x)}

    drawCount += 8

    if error <= 0 {
      y += 1
      error += ty
      ty += 2
    }
    if error > 0 {
      x -= 1
      tx += 2
      error += tx - diameter
    }
  }

  sdl3.SetRenderDrawColor(renderer, 255, 0,  0, 255)
  assert(sdl3.RenderLines(renderer, points, arrSize) == true,strings.clone_from_cstring(sdl3.GetError()))
}

normalizeAngle::proc(angle: f64) -> f64 {
  return angle - (2 * math.PI * math.floor(angle / (2*math.PI)))
}

cast_rays::proc(renderer: ^sdl3.Renderer, player: ^Player) {
  rayAngle := player.rotationAngle - (FOV_ANGLE/2)
  for i in 0..<NUM_RAYS {
    newRay := normalizeAngle(rayAngle)
    isRayDown := newRay > 0 && newRay < math.PI
    isRayRight := newRay < 0.5 * math.PI || newRay > 1.5 * math.PI

    // horizontal interception
    horizontal_y_intercept := f64(int(player.y/TILE_SIZE)*TILE_SIZE) + (TILE_SIZE if isRayDown else 0)
    horizontal_x_intercept := player.x+(horizontal_y_intercept-player.y)/math.tan(newRay)

    horizontal_x_step := TILE_SIZE/math.tan(newRay)
    horizontal_x_step *= (-1 if !isRayRight && horizontal_x_step > 0 else 1)
    horizontal_x_step *= (-1 if isRayRight && horizontal_x_step < 0 else 1)
    horizontal_y_step := f64(TILE_SIZE * (-1 if !isRayDown else 1))

    next_horizontal_touch_x := horizontal_x_intercept
    next_horizontal_touch_y := horizontal_y_intercept
    
    horizontal_wall_hit_x := f64(0)
    horizontal_wall_hit_y := f64(0)

    for next_horizontal_touch_x >= 0 && next_horizontal_touch_x <= WINDOW_WIDTH && next_horizontal_touch_y >= 0 && next_horizontal_touch_y <= WINDOW_HEIGHT{
      if (walls[int((next_horizontal_touch_y-(1 if !isRayDown else 0))/TILE_SIZE)*MAP_NUM_COLS+int(next_horizontal_touch_x/TILE_SIZE)] == 1) {
        horizontal_wall_hit_x = next_horizontal_touch_x
        horizontal_wall_hit_y = next_horizontal_touch_y
        break
      }
      else {
        next_horizontal_touch_x += horizontal_x_step
        next_horizontal_touch_y += horizontal_y_step
      }
    }

    // vertical_interception
    vertical_x_intercept := f64(int(player.x/TILE_SIZE)*TILE_SIZE) + (TILE_SIZE if isRayRight else 0)
    vertical_y_intercept := player.y+(vertical_x_intercept-player.x)*math.tan(newRay)

    vertical_y_step := TILE_SIZE*math.tan(newRay)
    vertical_y_step *= (-1 if !isRayDown && vertical_y_step > 0 else 1)
    vertical_y_step *= (-1 if isRayDown && vertical_y_step < 0 else 1)
    vertical_x_step := f64(TILE_SIZE * (-1 if !isRayRight else 1))

    next_vertical_touch_x := vertical_x_intercept
    next_vertical_touch_y := vertical_y_intercept

    vertical_wall_hit_x := f64(0)
    vertical_wall_hit_y := f64(0)

    for next_vertical_touch_x >= 0 && next_vertical_touch_x <= WINDOW_WIDTH && next_vertical_touch_y >= 0 && next_vertical_touch_y <= WINDOW_HEIGHT{
      if (walls[int(next_vertical_touch_y/TILE_SIZE)*MAP_NUM_COLS+int((next_vertical_touch_x-(1 if !isRayRight else 0))/TILE_SIZE)] == 1) {
        vertical_wall_hit_x = next_vertical_touch_x
        vertical_wall_hit_y = next_vertical_touch_y
        break
      }
      else {
        next_vertical_touch_x += vertical_x_step
        next_vertical_touch_y += vertical_y_step
      }
    }

    horizontal_hit_distance := math.sqrt(math.pow(player.x - horizontal_wall_hit_x, 2) + math.pow(player.y - horizontal_wall_hit_y, 2))
    vertical_hit_distance := math.sqrt(math.pow(player.x - vertical_wall_hit_x, 2) + math.pow(player.y - vertical_wall_hit_y, 2))

    res_x, res_y, distance : f64

    if horizontal_hit_distance < vertical_hit_distance {
      res_x = horizontal_wall_hit_x
      res_y = horizontal_wall_hit_y
      distance = horizontal_hit_distance
    }
    else { 
      res_x = vertical_wall_hit_x
      res_y = vertical_wall_hit_y
      distance = vertical_hit_distance
    }

    // render minimap_ray
    sdl3.RenderLine(renderer, f32(player.x)*MINIMAP_SCALE_FACTOR, f32(player.y)*MINIMAP_SCALE_FACTOR, f32(res_x)*MINIMAP_SCALE_FACTOR, f32(res_y)*MINIMAP_SCALE_FACTOR)

    // render actuall wall
    distance_to_projection_plane := (WINDOW_WIDTH/2) / math.tan(f64(FOV_ANGLE/2))
    wall_strip_height := (TILE_SIZE/distance) * distance_to_projection_plane;

    sdl3.RenderFillRect(renderer, &sdl3.FRect{f32( i*WALL_STRIP_WIDTH ), f32((WINDOW_HEIGHT/2)-(wall_strip_height/2)), WALL_STRIP_WIDTH, f32(wall_strip_height)})
  
    rayAngle += FOV_ANGLE / NUM_RAYS
  }
}

main::proc() {
  assert(sdl3.Init(sdl3.INIT_VIDEO) == true, strings.clone_from_cstring(sdl3.GetError()))
  defer sdl3.Quit()

  window := sdl3.CreateWindow("raycaster", WINDOW_WIDTH, WINDOW_HEIGHT, sdl3.WINDOW_BORDERLESS);
  defer sdl3.DestroyWindow(window);

  renderer := sdl3.CreateRenderer(window, nil)
  assert(renderer != nil, strings.clone_from_cstring(sdl3.GetError()))
  defer sdl3.DestroyRenderer(renderer)

  player := Player{WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 16, 0, 0, math.PI/2, 2, 2 * (math.PI/180)}


  for {
    event: sdl3.Event
    for sdl3.PollEvent(&event) {
      #partial switch event.type {
      case .QUIT:
        return
      case .KEY_DOWN:
        if event.key.scancode == sdl3.Scancode.ESCAPE do return
        if event.key.scancode == sdl3.Scancode.UP do player.walkDirection = 1
        if event.key.scancode == sdl3.Scancode.DOWN do player.walkDirection = -1
        if event.key.scancode == sdl3.Scancode.LEFT do player.turnDirection = -1
        if event.key.scancode == sdl3.Scancode.RIGHT do player.turnDirection = 1
      case .KEY_UP:
        if event.key.scancode == sdl3.Scancode.ESCAPE do return
        if event.key.scancode == sdl3.Scancode.UP do player.walkDirection = 0
        if event.key.scancode == sdl3.Scancode.DOWN do player.walkDirection = 0
        if event.key.scancode == sdl3.Scancode.LEFT do player.turnDirection = 0
        if event.key.scancode == sdl3.Scancode.RIGHT do player.turnDirection = 0
      }
    }
    player.rotationAngle += player.turnDirection * player.rotationSpeed
    moveStep := f64(player.walkDirection * player.moveSpeed)

    newX := player.x + math.cos(player.rotationAngle) * moveStep
    newY := player.y + math.sin(player.rotationAngle) * moveStep

    if walls[int(newY/TILE_SIZE)*MAP_NUM_COLS+int(newX/TILE_SIZE)] == 0 {
      player.x = newX
      player.y = newY
    }

    sdl3.RenderClear(renderer)

    for i in 0..<MAP_NUM_ROWS {
      for j in 0..<MAP_NUM_COLS {
        if (walls[i*MAP_NUM_COLS+j]) == 1 {
          sdl3.SetRenderDrawColor(renderer, 0, 0, 0, 255)
        }
        else {
          sdl3.SetRenderDrawColor(renderer, 255, 255, 255, 255)
        }
        sdl3.RenderFillRect(renderer, &sdl3.FRect{MINIMAP_SCALE_FACTOR*f32(j*TILE_SIZE), MINIMAP_SCALE_FACTOR*f32(i*TILE_SIZE), MINIMAP_SCALE_FACTOR*TILE_SIZE, MINIMAP_SCALE_FACTOR*TILE_SIZE})
      }
    }

    render_circle(renderer, MINIMAP_SCALE_FACTOR*player.x, MINIMAP_SCALE_FACTOR*player.y, int(MINIMAP_SCALE_FACTOR*player.radius))
    sdl3.SetRenderDrawColor(renderer, 0, 0, 255, 255)
    cast_rays(renderer, &player)
    sdl3.SetRenderDrawColor(renderer, 255, 255, 255, 255)
    assert(sdl3.RenderPresent(renderer) == true, strings.clone_from_cstring(sdl3.GetError()))
  }
}
