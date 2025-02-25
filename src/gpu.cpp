#include "gpu.h"

#include <SDL2/SDL.h>

Gpu::Gpu(Memory<std::istream, std::ostream, std::ostream> const& memory)
    : window_(SDL_CreateWindow("SLUG", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, WindowWidth,
                               WindowHeight, 0)),
      surface_(SDL_GetWindowSurface(window_)),
      toMemory_(memory),
      pixelFormat_(SDL_AllocFormat(SDL_GetWindowPixelFormat(window_)))

{}

Gpu::~Gpu() {
  SDL_DestroyWindow(window_);
  SDL_FreeFormat(pixelFormat_);
}

// gets the pixel address for a point in the window
address_t Gpu::getPixelAddress(const uint16_t& x, const uint16_t& y) {
  address_t pixel_index = x + (y * WindowWidth);
  address_t pixel_offset = 1 * pixel_index;
  return 0x3400 + pixel_offset;
}

// renders whatever is in the frame buffer in VRAM to the window
void Gpu::renderFrame() {
  SDL_LockSurface(surface_);
  address_t pixelAdr;
  byte_t pixel;
  uint8_t pixelColor;
  uint32_t** surfacePixals = (uint32_t**)(&(surface_->pixels));

  for (uint16_t pixelX = 0; pixelX < WindowWidth; pixelX++) {
    for (uint16_t pixelY = 0; pixelY < WindowHeight; pixelY++) {
      // grab the pixel from the frame buffer
      pixelAdr = Gpu::getPixelAddress(pixelX, pixelY);
      pixel = toMemory_.read_byte(pixelAdr);

      // map that pixel onto the surface
      (*surfacePixals)[pixelX + (pixelY * WindowWidth)] =
          SDL_MapRGB(pixelFormat_, pixel, pixel, pixel);
    }
  }
  // render frame data to surface
  SDL_UnlockSurface(surface_);
  SDL_UpdateWindowSurface(window_);
}
