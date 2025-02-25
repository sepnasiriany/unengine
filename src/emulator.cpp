#include "emulator.h"

#include <SDL2/SDL.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>

#include "controller.h"
#include "gpu.h"
#include "instruction_data.h"
#include "rom.h"

using namespace std::chrono_literals;
#define FRAME_PERIOD 16.67ms

Register::Register() : is_zero_(false), value_(0) {}
Register::Register(bool is_zero) : is_zero_(is_zero), value_(0) {}
Register::Register(register_value_t value) : is_zero_(false), value_(value) {}

Register &Register::operator=(register_value_t value) {
  if (!is_zero_) {
    this->value_ = value;
  }

  return *this;
}

register_value_t Register::operator*() const {
  if (is_zero_) return 0;
  return value_;
}

void stop_emulator() { throw StopException(); }

Emulator::Emulator()
    : memory_(std::cin, std::cout, std::cerr, cont_, stop_emulator),
      gpu(memory_) {
  register_file_[0] = Register(true);

  for (int i = 1; i < 32; i++) {
    register_file_[i] = Register(false);
  }
}

void Emulator::handle_event(SDL_Event const &evt) {
  switch (evt.type) {
    case SDL_WINDOWEVENT:
      if (evt.window.event == SDL_WINDOWEVENT_CLOSE) {
        warn("closing window due to quit");
        throw StopException();
      }
      break;
    case SDL_KEYDOWN:
      switch (evt.key.keysym.sym) {
        case SDLK_RETURN:
          cont_.push_button(START);
          break;
        case SDLK_SPACE:
          cont_.push_button(SELECT);
          break;
        case SDLK_UP:
          cont_.push_button(UP);
          break;
        case SDLK_DOWN:
          cont_.push_button(DOWN);
          break;
        case SDLK_LEFT:
          cont_.push_button(LEFT);
          break;
        case SDLK_RIGHT:
          cont_.push_button(RIGHT);
          break;
        case SDLK_z:
          cont_.push_button(B);
          break;
        case SDLK_x:
          cont_.push_button(A);
          break;
      }
      break;
    case SDL_KEYUP:
      switch (evt.key.keysym.sym) {
        case SDLK_RETURN:
          cont_.unpush_button(START);
          break;
        case SDLK_SPACE:
          cont_.unpush_button(SELECT);
          break;
        case SDLK_UP:
          cont_.unpush_button(UP);
          break;
        case SDLK_DOWN:
          cont_.unpush_button(DOWN);
          break;
        case SDLK_LEFT:
          cont_.unpush_button(LEFT);
          break;
        case SDLK_RIGHT:
          cont_.unpush_button(RIGHT);
          break;
        case SDLK_z:
          cont_.unpush_button(B);
          break;
        case SDLK_x:
          cont_.unpush_button(A);
          break;
      }
      break;
  }
}

void Emulator::execute_rom(const Rom &rom) {
  memory_.mount_rom(rom);
  // initialize stack pointer
  register_file_[29] = 0x3400;

  register_file_[31] = 0x00;
  program_counter_ = rom.slug_setup_;

  // setup function
  while (program_counter_ != 0) {
    Instruction i{memory_.read_instruction(program_counter_)};
    // std::cerr << "s: " << i << std::endl;
    if (i.is_i_type()) {
      execute_I_Instruction(i.decode_i_type());
    } else if (i.is_r_type()) {
      execute_R_Instruction(i.decode_r_type());
    }
    // std::cerr << "PC: " << program_counter_ << std::endl;
  }

  std::chrono::steady_clock timer;

  // loop function
  for (;;) {
    program_counter_ = rom.slug_loop_;
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
      handle_event(evt);
    }
    auto start = timer.now();
    while (program_counter_ != 0) {
      Instruction i{memory_.read_instruction(program_counter_)};
      // std::cerr << "l: " << i << std::endl;
      if (i.is_i_type()) {
        execute_I_Instruction(i.decode_i_type());
      } else if (i.is_r_type()) {
        execute_R_Instruction(i.decode_r_type());
      }
    }

    auto end = timer.now();
    while (end - start < FRAME_PERIOD) {
      int millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                       FRAME_PERIOD - (end - start))
                       .count();
      bool is_event = SDL_WaitEventTimeout(&evt, millis);
      if (is_event) handle_event(evt);
      end = timer.now();
    }

    gpu.renderFrame();
  }
}

void Emulator::execute_I_Instruction(const ITypeInstruction &i) {
  register_value_t a = *register_file_[(int)i.reg_a];
  register_value_t b = *register_file_[(int)i.reg_b];
  immediate_t immediate = i.immediate;

  switch (static_cast<Opcode>(i.opcode)) {
    case Opcode::ORI:
      register_file_[i.reg_b] = a | immediate;
      break;
    case Opcode::ADDI:
      register_file_[i.reg_b] = a + immediate;
      break;
    case Opcode::BEQ:
      if (a == b) {
        program_counter_ += immediate * 4;
      }
      break;
    case Opcode::BNE:
      if (a != b) {
        program_counter_ += immediate * 4;
      }
      break;
    case Opcode::SB:
      memory_.write_byte(a + immediate, b & 0xFF);
      break;
    case Opcode::LBU:
      register_file_[i.reg_b] = memory_.read_byte(a + immediate);
      break;
    case Opcode::JAL:
      register_file_[31] = program_counter_ + 4;
      program_counter_ = immediate * 4;
      break;
    case Opcode::LW:
      register_file_[i.reg_b] = memory_.read_word(a + immediate);
      break;
    case Opcode::SW:
      memory_.write_word(a + immediate, b);
      break;
    case Opcode::J:
      program_counter_ = immediate * 4;
      break;
    default:
      warn("Non I-Type instruction!");
      break;
  }

  // Increment PC if it's not a jump instruction
  if (i.opcode != static_cast<uint8_t>(Opcode::JAL) &&
      i.opcode != static_cast<uint8_t>(Opcode::J)) {
    program_counter_ += 4;
  }
}

void Emulator::execute_R_Instruction(const RTypeInstruction &r) {
  if (r.opcode != static_cast<uint8_t>(Opcode::RTYPE)) {
    warn("Non R type instruction!");
    return;
  }

  register_value_t a = *register_file_[(int)r.reg_a];
  register_value_t b = *register_file_[(int)r.reg_b];

  switch (static_cast<FunctionCode>(r.function)) {
    case FunctionCode::NOR:
      register_file_[r.reg_c] = ~(a | b);
      break;
    case FunctionCode::SLT:
      // ISA expects these to be signed values
      register_file_[r.reg_c] = *reinterpret_cast<const int16_t *>(&a) <
                                *reinterpret_cast<const int16_t *>(&b);
      break;
    case FunctionCode::SLL:
      register_file_[r.reg_c] = (b << r.shift_value);
      break;
    case FunctionCode::SRA:
      register_file_[r.reg_c] =
          *(reinterpret_cast<const int16_t *>(&b)) >> r.shift_value;
      break;
    case FunctionCode::JR:
      program_counter_ = a;
      break;
    case FunctionCode::SRL:
      register_file_[r.reg_c] = b >> r.shift_value;
      break;
    case FunctionCode::OR:
      register_file_[r.reg_c] = a | b;
      break;
    case FunctionCode::SUB:
      register_file_[r.reg_c] = a - b;
      break;
    case FunctionCode::ADD:
      register_file_[r.reg_c] = a + b;
      break;
    case FunctionCode::AND:
      register_file_[r.reg_c] = a & b;
      break;

    default:
      warn("Non R type instruction!");
      break;
  }

  // if instruction is not jr, incr PC
  if (r.function != static_cast<uint8_t>(FunctionCode::JR))
    program_counter_ += 4;
}

void Emulator::save_state(const std::string &filename) {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error opening file for saving state." << std::endl;
    return;
  }

  file.write(reinterpret_cast<const char *>(&program_counter_),
             sizeof(program_counter_));

  for (int i = 0; i < NUM_REGISTERS; ++i) {
    register_value_t regVal = *register_file_[i];
    file.write(reinterpret_cast<const char *>(&regVal), sizeof(regVal));
  }

  file.write(reinterpret_cast<const char *>(memory_.get_memory_buffer()),
             memory_.get_memory_size());

  file.close();
}

void Emulator::load_state(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error opening file for loading state." << std::endl;
    return;
  }

  file.read(reinterpret_cast<char *>(&program_counter_),
            sizeof(program_counter_));

  for (int i = 0; i < NUM_REGISTERS; ++i) {
    register_value_t regVal;
    file.read(reinterpret_cast<char *>(&regVal), sizeof(regVal));
    register_file_[i] = regVal;
  }

  file.read(reinterpret_cast<char *>(memory_.get_memory_buffer()),
            memory_.get_memory_size());
  file.close();
}

register_value_t Emulator::get_register_value(const uint8_t &index) {
  return *register_file_[index];
}

MemoryIo &Emulator::get_memory() { return memory_; }

void Emulator::set_register_value(const uint8_t &index,
                                  const register_value_t &value) {
  register_file_[index] = value;
}

void Emulator::set_program_counter(const register_value_t &value) {
  program_counter_ = value;
}

register_value_t Emulator::get_program_counter() { return program_counter_; }
