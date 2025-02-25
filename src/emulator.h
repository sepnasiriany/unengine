#pragma once

#include <SDL2/SDL.h>

#include <iostream>

#include "controller.h"
#include "gpu.h"
#include "instruction.h"
#include "memory.h"
#include "rom.h"
#include "types.h"

const int NUM_REGISTERS = 32;

class Register {
 private:
  bool is_zero_;
  register_value_t value_;

 public:
  Register();
  Register(bool is_zero);
  Register(register_value_t value);

  register_value_t operator*() const;
  Register &operator=(register_value_t val);
};

using MemoryIo = Memory<std::istream, std::ostream, std::ostream>;

class StopException {};

class Emulator {
 private:
  Register register_file_[NUM_REGISTERS];
  register_value_t program_counter_;
  MemoryIo memory_;
  ControllerState cont_;
  Gpu gpu;

  void handle_event(const SDL_Event &evt);

 public:
  Emulator();

  register_value_t get_register_value(const uint8_t &index);
  void set_register_value(const uint8_t &index, const register_value_t &value);
  void set_program_counter(const register_value_t &value);
  register_value_t get_program_counter();
  void execute_rom(const Rom &rom);

  void execute_I_Instruction(const ITypeInstruction &i);
  void execute_R_Instruction(const RTypeInstruction &r);
  void save_state(const std::string &filename);
  void load_state(const std::string &filename);
  MemoryIo &get_memory();

  // for testing
  friend int main(int, char **);
};
