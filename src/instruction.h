#pragma once

#include "types.h"

struct ITypeInstruction {
 public:
  opcode_t opcode;
  register_index_t reg_a;
  register_index_t reg_b;
  immediate_t immediate;
};

struct RTypeInstruction {
 public:
  opcode_t opcode;
  register_index_t
      reg_a;  // Reminder these are indicies into register array [0-31]
  register_index_t reg_b;
  register_index_t reg_c;
  std::uint8_t shift_value;
  opcode_t function;
};

// TODO: implement this using inheritance
struct Instruction {
 public:
  instruction_t encoded_value;

  opcode_t opcode() const;

  bool is_i_type() const;
  bool is_r_type() const;

  ITypeInstruction decode_i_type() const;
  RTypeInstruction decode_r_type() const;
};

#ifndef RELEASE
#include <iostream>
std::ostream &operator<<(std::ostream &os, Instruction i);
#endif
