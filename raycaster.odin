package raycaster

import "vendor:sdl3"
import "core:fmt" 
import "core:strings"
main::proc() {
  fmt.println("Hello")
  assert(sdl3.Init(sdl3.INIT_VIDEO) == true, strings.clone_from_cstring(sdl3.GetError()))
  defer sdl3.Quit()

  window := sdl3.CreateWindow("raycaster", 640, 640, sdl3.WINDOW_BORDERLESS);
  defer sdl3.DestroyWindow(window);

  renderer := sdl3.CreateRenderer(window, nil)
  assert(renderer != nil, strings.clone_from_cstring(sdl3.GetError()))
  defer sdl3.DestroyRenderer(renderer)

  rect := sdl3.FRect{20, 20, 20, 20}

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
    sdl3.SetRenderDrawColor(renderer, 0, 0, 128, 255)
    sdl3.RenderFillRect(renderer, &rect)

    assert(sdl3.RenderPresent(renderer) == true, strings.clone_from_cstring(sdl3.GetError()))
  }
}
