#include <cstdio>
#include <iostream>

#include "instruction.h"
#include "rom.h"
#include "types.h"

using std::printf;

void disassemble(const Rom &r, slug_space_t start, slug_space_t end) {
  slug_space_t pc = start;
  for (instruction_t i; pc < end; pc += 4) {
    i = r.read32(pc);
    if (!i) continue;

    Instruction in{i};
    if (in.is_i_type()) {
      auto i = in.decode_i_type();
      printf("%x: (%2d): ", pc, i.opcode);
      switch (i.opcode) {
        case 0:
          printf("ori x%d <- x%d | %d", i.reg_b, i.reg_a, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 10:
          printf("addi x%d <- x%d + %d", i.reg_b, i.reg_a, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 12:
          printf("beq x%d == x%d => +%d", i.reg_a, i.reg_b, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 25:
          printf("bne x%d == x%d => +%d", i.reg_a, i.reg_b, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 30:
          printf("sb x%d => x%d[%d]", i.reg_b, i.reg_a, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 40:
          printf("lbu x%d <- x%d[%d]", i.reg_b, i.reg_a, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 51:
          printf("jal %d", i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 55:
          printf("lw x%d <- x%d[%d]", i.reg_b, i.reg_a, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 60:
          printf("sw x%d => x%d[%d]", i.reg_b, i.reg_a, i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
        case 61:
          printf("j %d", i.immediate);
          if (i.immediate > 100) printf(" (hex %x)", i.immediate);
          break;
      }
    } else if (in.is_r_type()) {
      auto n = in.decode_r_type();
      printf("%x: (%2d): ", pc, n.function);
      switch (n.function) {
        case 0:
          printf("nor x%d <- x%d ~| x%d", n.reg_c, n.reg_a, n.reg_b);
          break;
        case 10:
          printf("slt x%d <- x%d < x%d", n.reg_c, n.reg_a, n.reg_b);
          break;
        case 13:
          printf("sll x%d <- x%d << %d", n.reg_c, n.reg_b, n.shift_value);
          break;
        case 16:
          printf("sra x%d <- x%d << %d", n.reg_c, n.reg_b, n.shift_value);
          break;
        case 22:
          printf("jr <- x%d", n.reg_a);
          break;
        case 25:
          printf("srl x%d <- x%d << %d", n.reg_c, n.reg_b, n.shift_value);
          break;
        case 31:
          printf("or x%d <- x%d | x%d", n.reg_c, n.reg_a, n.reg_b);
          break;
        case 33:
          printf("sub x%d <- x%d - x%d", n.reg_c, n.reg_a, n.reg_b);
          break;
        case 34:
          printf("add x%d <- x%d + x%d", n.reg_c, n.reg_a, n.reg_b);
          break;
        case 58:
          printf("and x%d <- x%d & x%d", n.reg_c, n.reg_a, n.reg_b);
          break;
        default:
          continue;
      }
    } else
      continue;
    printf("\n");
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " <rom file> [addresses]."
              << std::endl;
    return 1;
  }

  Rom r = Rom::ReadRomFile(argv[1]);
  slug_space_t setup = r.slug_setup_ - 0x8000, loop = r.slug_loop_ - 0x8000,
               data = r.slug_load_data_address_ - 0x8000;
  slug_space_t pc = setup;

  std::cout << "setup(): " << std::endl;
  disassemble(r, setup, loop);

  std::cout << "loop(): " << std::endl;
  disassemble(r, loop, data);

  std::cout << "data section: " << std::endl;
  for (slug_space_t i = 0; i < r.slug_size_; i += 16) {
    std::printf("0x%x (0x%x):  ", r.slug_program_data_address_ + i, data + i);
    for (slug_space_t j = 0; j < 16 && i + j < r.slug_size_; j++) {
      // std::printf("%02hhX ", r.read_byte_(data + i));
    }
    std::printf("\n");
  }

  disassemble(r, 0x1d8, setup);
}
