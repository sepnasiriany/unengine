enum class Opcode : uint8_t {

  // For I-type instructions
  ORI = 0,
  ADDI = 10,
  BEQ = 12,
  BNE = 25,
  RTYPE = 28,
  SB = 30,
  LBU = 40,
  JAL = 51,
  LW = 55,
  SW = 60,
  J = 61,
};

enum class FunctionCode : uint8_t {

  // For R-type instructions
  NOR = 0,
  SLT = 10,
  SLL = 13,
  SRA = 16,
  JR = 22,
  SRL = 25,
  OR = 31,
  SUB = 33,
  ADD = 34,
  AND = 58
};
