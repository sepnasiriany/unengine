#include "instruction.h"

#include "instruction_data.h"

// First 6 bit mask
uint32_t mask_6_bit = 0xFC000000;

opcode_t Instruction::opcode() const {
  uint8_t optCode = (encoded_value & mask_6_bit) >> (32 - 6);

  return optCode;
}

bool Instruction::is_i_type() const {
  uint8_t optCode = (encoded_value & mask_6_bit) >> (32 - 6);

  return optCode != static_cast<uint8_t>(Opcode::RTYPE);
}
bool Instruction::is_r_type() const {
  uint8_t optCode = (encoded_value & mask_6_bit) >> (32 - 6);

  return optCode == static_cast<uint8_t>(Opcode::RTYPE);
}

ITypeInstruction Instruction::decode_i_type() const {
  ITypeInstruction new_instruction;

  // Get Optcode >[6]< [5][5][16]
  uint8_t optCode = (encoded_value & mask_6_bit) >> (32 - 6);
  new_instruction.opcode = (opcode_t)optCode;

  // Get reg_a [6] >[5]< [5][16]
  new_instruction.reg_a = (encoded_value & 0x03E00000) >> (32 - 11);

  // Get reg_b [6][5] >[5]< [16]
  new_instruction.reg_b = (encoded_value & 0x001F0000) >> (32 - 16);

  // Get immediate [6][5][5] >[16]<
  new_instruction.immediate = (encoded_value & 0x0000FFFF);

  return new_instruction;
}

RTypeInstruction Instruction::decode_r_type() const {
  RTypeInstruction new_instruction;

  // Get Optcode >[6]< [5][5][5][5][6]
  uint8_t optCode = (encoded_value & mask_6_bit) >> (32 - 6);
  new_instruction.opcode = (opcode_t)optCode;

  // Get reg_a [6] >[5]< [5][5][5][6]
  new_instruction.reg_a = (encoded_value & 0x03E00000) >> (32 - 11);

  // Get reg_b [6][5] >[5]< [5][5][6]
  new_instruction.reg_b = (encoded_value & 0x001F0000) >> (32 - 16);

  // Get reg_c [6][5][5] >[5]< [5][6]
  new_instruction.reg_c = (encoded_value & 0x0000F800) >> (32 - 21);

  // Get shift value [6][5][5][5] >[5]< [6]
  new_instruction.shift_value = (encoded_value & 0x000007C0) >> (32 - 26);

  // Get function [6][5][5][5][5] >[6]<
  new_instruction.function = (encoded_value & 0x0000003F);

  return new_instruction;
}

#ifndef RELEASE
std::ostream &operator<<(std::ostream &os, Instruction i) {
  if (i.is_i_type()) {
    auto n = i.decode_i_type();
    os << "i-type { op: " << (int)i.opcode() << "; ra: " << (int)n.reg_a
       << "; rb: " << (int)n.reg_b << "; imm: " << (int)n.immediate << " }";
  } else if (i.is_r_type()) {
    auto n = i.decode_r_type();
    os << "r-type { op: " << (int)i.opcode() << "; ra: " << (int)n.reg_a
       << "; rb: " << (int)n.reg_b << "; rc: " << (int)n.reg_c
       << "; fn: " << (int)n.function << "; sh: " << (int)n.shift_value << " }";
  } else {
    os << "Invalid Instruction { " << std::hex << i << " (opcode "
       << (int)i.opcode() << ") }";
  }
  return os;
}
#endif
