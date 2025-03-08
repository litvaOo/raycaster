package raycaster

import "vendor:sdl3"
import "core:fmt" 
import "core:strings"

main::proc() {
  assert(sdl3.Init(sdl3.INIT_VIDEO) == true, strings.clone_from_cstring(sdl3.GetError()))
  defer sdl3.Quit()

  TILE_SIZE :: 32
  MAP_NUM_ROWS :: 11
  MAP_NUM_COLS :: 15

  window := sdl3.CreateWindow("raycaster", MAP_NUM_COLS*TILE_SIZE, MAP_NUM_ROWS*TILE_SIZE, sdl3.WINDOW_BORDERLESS);
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
    sdl3.SetRenderDrawColor(renderer, 0, 0, 0, 255)
    assert(sdl3.RenderPresent(renderer) == true, strings.clone_from_cstring(sdl3.GetError()))
  }
}
