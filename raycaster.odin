package raycaster

import "vendor:sdl3"
import "core:fmt" 
import "core:strings"
import "core:math"

render_circle::proc(renderer: ^sdl3.Renderer, center_x: int, center_y: int, radius: int) {
  arrSize := i32(((radius * 8 * 35 / 49) + (8 - 1)) & -8)
  points := make([^]sdl3.FPoint, arrSize)
  drawCount := 0
  diameter := radius * 2

  x := radius - 1
  y := 0

  tx := 1
  ty := 1

  error := tx - diameter

  for x >= y {
    points[drawCount+0] = {f32( center_x + x ), f32(center_y - y)}
    points[drawCount+1] = {f32( center_x + x ), f32(center_y + y)}
    points[drawCount+2] = {f32( center_x - x ), f32(center_y - y)}
    points[drawCount+3] = {f32( center_x - x ), f32(center_y + y)}
    points[drawCount+4] = {f32( center_x + x ), f32(center_y - y)}
    points[drawCount+5] = {f32( center_x + x ), f32(center_y + y)}
    points[drawCount+6] = {f32( center_x - x ), f32(center_y - y)}
    points[drawCount+7] = {f32( center_x - x ), f32(center_y + y)}

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

  sdl3.SetRenderDrawColor(renderer, 0, 255, 0, 255)
  assert(sdl3.RenderPoints(renderer, points, arrSize) == true,strings.clone_from_cstring(sdl3.GetError()))
  sdl3.SetRenderDrawColor(renderer, 0, 0, 0, 255)
}


main::proc() {
  assert(sdl3.Init(sdl3.INIT_VIDEO) == true, strings.clone_from_cstring(sdl3.GetError()))
  defer sdl3.Quit()

  TILE_SIZE :: 32
  MAP_NUM_ROWS :: 11
  MAP_NUM_COLS :: 15

  WINDOW_WIDTH :: MAP_NUM_COLS*TILE_SIZE 
  WINDOW_HEIGHT :: MAP_NUM_ROWS*TILE_SIZE 
  window := sdl3.CreateWindow("raycaster", WINDOW_WIDTH, WINDOW_HEIGHT, sdl3.WINDOW_BORDERLESS);
  defer sdl3.DestroyWindow(window);

  renderer := sdl3.CreateRenderer(window, nil)
  assert(renderer != nil, strings.clone_from_cstring(sdl3.GetError()))
  defer sdl3.DestroyRenderer(renderer)

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

  Player::struct {
    x: int, 
    y: int,
    radius: int,
    turnDirection: int, // -1 if left, +1 if right
    walkDirection: int, // -1 if back, +1 if right
    rotationAngle: f64,
    moveSpeed: int,
    rotationSpeed: f64
  }
  player := Player{WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 20, 0, 0, math.PI/2, 2, 2 * (math.PI/180)}

  for {
    event: sdl3.Event
    for sdl3.PollEvent(&event) {
      #partial switch event.type {
      case .QUIT:
        return
      case .KEY_DOWN:
        return
      }
    }
    sdl3.RenderClear(renderer)
    for i in 0..<MAP_NUM_ROWS {
      for j in 0..<MAP_NUM_COLS {
        sdl3.SetRenderDrawColor(renderer, 255*(walls[i*MAP_NUM_COLS+j]), 0, 0, 255)
        sdl3.RenderFillRect(renderer, &sdl3.FRect{f32(j*32.0), f32(i*32.0), TILE_SIZE, TILE_SIZE})
      }
    }
    render_circle(renderer, player.x, player.y, player.radius)
    sdl3.SetRenderDrawColor(renderer, 0, 0, 0, 255)
    assert(sdl3.RenderPresent(renderer) == true, strings.clone_from_cstring(sdl3.GetError()))
  }
}
