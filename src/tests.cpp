#include <gtest/gtest.h>

#include <sstream>

#include "controller.h"
#include "emulator.h"
#include "instruction.h"
#include "instruction_data.h"
#include "memory.h"
#include "rom.h"
#include "types.h"

TEST(RegisterTests, BasicFunctionality) {
  Register zero(true), writable(static_cast<register_value_t>(0));

  zero = 5;
  writable = 5;

  ASSERT_EQ(*zero, 0) << "Zero register should not be mutable.";
  ASSERT_EQ(*writable, 5) << "Writable register should be mutable.";
}

TEST(MemoryTests, Memory) {
  std::istringstream in("In!");
  std::ostringstream out, err;
  ControllerState cont;

  bool stopped = false;
  bool *p = &stopped;

  Memory<typeof(in), typeof(out), typeof(err)> mem(in, out, err, cont,
                                                   [p]() { *p = true; });

  // this takes a BE number.
  mem.write_word(0x4000, htons(0x5678));
  ASSERT_EQ(mem.read_byte(0x4000), 0x78);
  ASSERT_EQ(mem.read_byte(0x4001), 0x56);

  mem.write_byte(0x7110, 'O');
  mem.write_byte(0x7120, 'E');
  mem.write_byte(0x7200, 'S');
  ASSERT_EQ(mem.read_byte(0x7100), 'I');
  ASSERT_EQ(out.str()[0], 'O');
  ASSERT_EQ(err.str()[0], 'E');
  ASSERT_EQ(stopped, true);
}

TEST(InstructionTests, IType) {
  // Instruction I example
  // Optcode: 25 [6]
  // reg_a: 15 [5]
  // reg_b: 0 [5]
  // Immediate: 65,535 [16]
  // [0000 19]
  // [0110 01][01 111][0 0000] [1111 1111 1111 1111]]
  // 0x65E0FFFF

  Instruction test;
  test.encoded_value = 0x65E0FFFF;

  ASSERT_EQ(test.is_i_type(), true);
  ASSERT_EQ(test.is_r_type(), false);
  ASSERT_EQ(test.opcode(), 25);

  ITypeInstruction decoded = test.decode_i_type();

  ASSERT_EQ(decoded.opcode, 25);
  ASSERT_EQ(decoded.reg_a, 15);
  ASSERT_EQ(decoded.reg_b, 0);
  ASSERT_EQ(decoded.immediate, 65535);
}

TEST(InstructionTests, RType) {
  // Instruction R Example
  // Optcode: 28
  // reg_a: 31
  // reg_b: 31
  // reg_c: 9
  // Shift: 14
  // function: 16
  // [0111 00][11 111][1 1111][0100 1][011 10][01 0000]
  // 0x73FF4B90

  Instruction test;
  test.encoded_value = 0x73FF4B90;

  ASSERT_EQ(test.is_i_type(), false);
  ASSERT_EQ(test.is_r_type(), true);
  ASSERT_EQ(test.opcode(), 28);

  RTypeInstruction decoded = test.decode_r_type();

  ASSERT_EQ(decoded.opcode, 28);
  ASSERT_EQ(decoded.reg_a, 31);
  ASSERT_EQ(decoded.reg_b, 31);
  ASSERT_EQ(decoded.reg_c, 9);
  ASSERT_EQ(decoded.shift_value, 14);
  ASSERT_EQ(decoded.function, 16);
}

TEST(RomTest, testRom) {
  Rom testrom = Rom::ReadRomFile("../rom-archive/hws/hello_world1.slug");
  // for (int i = 0; i < 4; i++) {
  //   std::cout << testrom.read_byte_(i) << std::endl;
  // }
  ASSERT_EQ(testrom.read32(0x0000),
            htonl(*reinterpret_cast<uint32_t const *>("SLUG")));
}

TEST(RomTest, readInstruction) {
  Rom testrom = Rom::ReadRomFile("../rom-archive/hws/hello_world1.slug");
  // std::cout << (int)testrom.read_byte_(testrom.slug_setup_) << std::endl;
  // std::cout << (int)testrom.read_byte_(testrom.slug_setup_+1) << std::endl;
  // std::cout << (int)testrom.read_byte_(testrom.slug_setup_+2) << std::endl;
  // std::cout << (int)testrom.read_byte_(testrom.slug_setup_+3) << std::endl;

  ASSERT_EQ((instruction_t)testrom.slug_setup_, (uint32_t)0x8280);
}

// opcode_t opcode;
// register_index_t reg_a;  // Reminder these are indicies into register array
// [0-31] register_index_t reg_b; register_index_t reg_c; std::uint8_t
// shift_value; opcode_t function;

TEST(InstructionTests_R, RType) {
  Emulator emu;
  RTypeInstruction current_instruction;

  int functions[10] = {0, 10, 13, 16, 22, 25, 31, 33, 34, 58};

  int expected_out[10] = {};

  // Set our tests values
  uint16_t a = 10;
  uint16_t b = 5;
  uint16_t out = 8;
  uint16_t shift = 2;

  current_instruction.opcode = 28;
  current_instruction.reg_a = 1;
  current_instruction.reg_b = 2;
  current_instruction.reg_c = 3;
  current_instruction.shift_value = 2;

  for (int i = 0; i < 10; i++) {
    // reset our register values
    emu.set_register_value(1, a);
    emu.set_register_value(2, b);
    emu.set_register_value(3, out);

    // Set function
    current_instruction.function = functions[i];

    // Execute R Instruction
    emu.execute_R_Instruction(current_instruction);

    // Test value

    register_value_t answer = (emu.get_register_value(3));

    switch (static_cast<FunctionCode>(functions[i])) {
      case FunctionCode::NOR:  // ~(a | b);
        ASSERT_EQ(answer, (uint16_t) ~(a | b));
        break;
      case FunctionCode::SLT:  // a < b;
        ASSERT_EQ(answer, (a < b));
        break;
      case FunctionCode::SLL:  // b << shift;
        ASSERT_EQ(answer, (b << shift));
        break;
      case FunctionCode::SRA:  // (int16_t) b >> shift
        ASSERT_EQ(answer, ((int16_t)b >> shift));
        break;
      case FunctionCode::JR:  // a <- check pc
        ASSERT_EQ(emu.get_program_counter(), a);
        break;
      case FunctionCode::SRL:  // b >> shift
        ASSERT_EQ(answer, (b >> shift));
        break;
      case FunctionCode::OR:  // a | b
        ASSERT_EQ(answer, (a | b));
        break;
      case FunctionCode::SUB:  // a - b
        ASSERT_EQ(answer, (a - b));
        break;
      case FunctionCode::ADD:  // a + b
        ASSERT_EQ(answer, (a + b));
        break;
      case FunctionCode::AND:  // a & b
        ASSERT_EQ(answer, (a & b));
        break;
    }
  }
}

TEST(InstructionTests_I, IType) {
  Emulator emu;
  ITypeInstruction current_instruction;

  int opcodes[10] = {0, 10, 12, 25, 30, 40, 51, 55, 60, 61};

  uint16_t a = 10;
  uint16_t b = 5;
  int16_t immediate = 3;
  uint16_t result;

  current_instruction.reg_a = 1;
  current_instruction.reg_b = 2;
  current_instruction.immediate = immediate;

  for (int i = 0; i < 10; i++) {
    emu.set_register_value(1, a);
    emu.set_register_value(2, b);
    emu.set_program_counter(0);

    uint32_t expected_return_address = emu.get_program_counter() + 4;
    uint32_t expected_pc = immediate * 4;

    current_instruction.opcode = opcodes[i];

    emu.execute_I_Instruction(current_instruction);

    result = emu.get_register_value(current_instruction.reg_b);

    switch (static_cast<Opcode>(opcodes[i])) {
      case Opcode::ORI:
        ASSERT_EQ(result, (a | immediate));
        break;
      case Opcode::ADDI:
        ASSERT_EQ(result, (a + immediate));
        break;
      case Opcode::BEQ:
        if (a == b) {
          ASSERT_EQ(emu.get_program_counter(), immediate * 4 + 4);
        } else {
          ASSERT_NE(emu.get_program_counter(), immediate * 4 + 4);
        }
        break;
      case Opcode::BNE:
        if (a != b) {
          ASSERT_EQ(emu.get_program_counter(), immediate * 4 + 4);
        } else {
          ASSERT_NE(emu.get_program_counter(), immediate * 4 + 4);
        }
        break;
      case Opcode::SB:
        ASSERT_EQ(emu.get_memory().read_byte(a + immediate), b & 0xFF);
        break;
      case Opcode::LBU:
        ASSERT_EQ(result, emu.get_memory().read_byte(a + immediate) & 0xFF);
        break;
      case Opcode::JAL:
        ASSERT_EQ(emu.get_register_value(31), expected_return_address);
        ASSERT_EQ(emu.get_program_counter(), expected_pc);
        break;
      case Opcode::LW:
        ASSERT_EQ(result, emu.get_memory().read_word(a + immediate));
        break;
      case Opcode::SW:
        ASSERT_EQ(emu.get_memory().read_word(a + immediate), b);
        break;
      case Opcode::J:
        ASSERT_EQ(emu.get_program_counter(), immediate * 4);
        break;
    }
  }
}

TEST(EmulatorStateTest, SaveLoadState) {
  Emulator emu1;
  emu1.set_register_value(1, 1234);
  emu1.set_program_counter(0x1000);
  for (address_t addr = 0; addr < 100; addr += 4) {
    emu1.get_memory().write_word(addr, 0xbeef);
  }

  std::string filename = "emu_state_test.bin";
  emu1.save_state(filename);

  Emulator emu2;
  emu2.load_state(filename);

  for (int i = 0; i < NUM_REGISTERS; ++i) {
    // std::cout<<"emu1 register value is " << emu1.get_register_value(i) << "
    // and emu2 register value is " << emu2.get_register_value(i) << std::endl;
    ASSERT_EQ(emu1.get_register_value(i), emu2.get_register_value(i))
        << "Registers at index " << i << " do not match.";
  }

  for (address_t addr = 0; addr < 100; addr += 4) {
    // std::cout<<"emu2 memory value is " << emu2.get_memory().read_word(addr)
    // << " and emu2 memory value is " << emu2.get_memory().read_word(addr) <<
    // std::endl;
    ASSERT_EQ(emu1.get_memory().read_word(addr),
              emu2.get_memory().read_word(addr))
        << "Memory at address " << std::hex << addr << " does not match.";
  }

  ASSERT_EQ(emu1.get_program_counter(), emu2.get_program_counter())
      << "Program counters do not match.";

  std::remove(filename.c_str());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
