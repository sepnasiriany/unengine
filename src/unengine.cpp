#include <SDL2/SDL.h>

#include <iostream>

#include "emulator.h"
#include "gpu.h"
#include "rom.h"
#include "types.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <rom file>." << std::endl;
    return 1;
  }

  // std::cout << "pre-init" << std::endl;
  SDL_Init(SDL_INIT_VIDEO);
  // std::cout << "post-init" << std::endl;
  Emulator emu;
  Rom r = Rom::ReadRomFile(argv[1]);
  // std::cout << "pre-execute" << std::endl;
  try {
    emu.execute_rom(r);
  } catch (StopException e) {
    return 0;
  }
  // std::cout << "post-execute" << std::endl;
  return 0;
}
