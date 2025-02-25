#pragma once

#include <SDL2/SDL.h>

#include <iostream>

#include "memory.h"
#include "rom.h"
#include "types.h"

enum GpuConfig {
  WindowHeight = 120,
  WindowWidth = 128,
  WindowArea = WindowHeight * WindowWidth
};

class Gpu {
 private:
  SDL_Window* window_;
  SDL_Surface* surface_;
  Memory<std::istream, std::ostream, std::ostream> const& toMemory_;
  SDL_PixelFormat* pixelFormat_;

  static address_t getPixelAddress(const uint16_t& width,
                                   const uint16_t& height);

 public:
  Gpu(Memory<std::istream, std::ostream, std::ostream> const& memory);
  ~Gpu();
  void renderFrame();
};
