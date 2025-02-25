#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "controller.h"
#include "rom.h"
#include "types.h"

enum PermissionBits : uint8_t {
  Read = 1 << 0,
  Write = 1 << 1,
  Execute = 1 << 2,
};

using permission_t = uint8_t;

namespace AddressSpace {
enum Addresses : address_t {
  RamStart = 0x0000,
  RamSize = 0x7000,
  StackStart = 0x1400,
  StackSize = 0x2000,
  VramStart = 0x3400,
  VramEnd = 0x3c00,
  ControllerIo = 0x7000,
  Stdin = 0x7100,
  Stdout = 0x7110,
  Stderr = 0x7120,
  StopExecution = 0x7200,
  RomStart = 0x8000,
};
}

constexpr size_t MEMORY_SIZE = 0x10000;

/* On endianness:
 *
 * we are probably using little-endian on our system
 * so we need to store data in little-endian
 *
 * whenever we read from a SLUG program, we need to convert it from big-endian
 * to little-endian using ntoh and hton
 */
template <typename In, typename Err, typename Out>
class Memory {
 private:
  In &in_;
  Out &out_;
  Err &err_;
  ControllerState &controller_;

  stopper_fn_t stopper_;

  std::unique_ptr<byte_t[]> buffer_;

  Memory() = delete;

 protected:
  permission_t perms_at_address(address_t a) const {
    using namespace AddressSpace;
    if (a < RamStart + RamSize) return Read | Write;
    if (a == ControllerIo) return Read;
    if (a == Stdin) return Read;
    if (a == Stdout) return Write;
    if (a == Stderr) return Write;
    if (a == StopExecution) return Write;
    if (a >= RomStart) return Read | Execute;
    return 0;
  }

 public:
  Memory(In &in, Out &out, Err &err, ControllerState &controller,
         const stopper_fn_t &stopper)
      : in_(in),
        out_(out),
        err_(err),
        controller_(controller),
        stopper_(stopper),
        buffer_(new byte_t[MEMORY_SIZE]()) {}

  const byte_t *get_memory_buffer() const { return buffer_.get(); }

  byte_t *get_memory_buffer() { return buffer_.get(); }

  size_t get_memory_size() const { return MEMORY_SIZE; }

  byte_t read_byte(address_t a) const {
    using namespace AddressSpace;
    if ((perms_at_address(a) & Read) == 0) {
      warn("Invalid read, returning 0.");
      return 0;
    }

    if (a == ControllerIo) return controller_.state();
    if (a == Stdin) {
      byte_t b;
      in_ >> b;
      return b;
    }

    return buffer_.get()[a];
  }

  word_t read_word(address_t a) const {
    using namespace AddressSpace;
    if ((perms_at_address(a) & Read) == 0) {
      warn("Invalid read, returning 0.");
      return 0;
    }
    if (a == Stdin || a == ControllerIo) {
      warn("Invalid word-read on byte IO, returning 0.");
      return 0;
    }

    address_t aligned_address = a / sizeof(word_t);

    return ntohs((reinterpret_cast<word_t *>(buffer_.get()))[aligned_address]);
  }

  void write_byte(address_t a, byte_t byte) {
    using namespace AddressSpace;
    if ((perms_at_address(a) & Write) == 0) {
      warn("Invalid write, performing nop.");
      return;
    }

    if (a == Stdout) {
      out_ << byte;
      return;
    }

    if (a == Stderr) {
      err_ << byte;
      return;
    }

    if (a == StopExecution) {
      stopper_();
      return;
    }

    buffer_.get()[a] = byte;
  }

  void write_word(address_t a, word_t word) {
    using namespace AddressSpace;
    if ((perms_at_address(a) & Write) == 0) {
      warn("Invalid write, performing nop.");
      return;
    }

    if (a == Stdout || a == Stderr || a == StopExecution) {
      warn("Invalid word-size IO write; performing nop");
      return;
    }

    address_t aligned_address = a / sizeof(word_t);
    reinterpret_cast<word_t *>(buffer_.get())[aligned_address] = htons(word);
  }

  /*
   * TODO: is this right?
   * this returns a little-endian instruction (32-bit uint)
   * because our instruction decoding depends on it being
   * little endian
   */
  instruction_t read_instruction(address_t a) const {
    if ((perms_at_address(a) & (Read | Execute)) == 0) {
      warn("read/execute not set for load-instruction, returning 0.");
      return 0;
    }

    address_t aligned_address = a / sizeof(instruction_t);

    return ntohl(reinterpret_cast<const instruction_t *>(
        buffer_.get())[aligned_address]);
  }

  void mount_rom(const Rom &rom) {
    std::memcpy(buffer_.get() + AddressSpace::RomStart, rom.contents().get(),
                SLUGValues::FILE_SIZE);

    // copy rom RAM to RAM
    std::memcpy(buffer_.get() + rom.slug_program_data_address_,
                rom.contents().get() + rom.slug_load_data_address_ - 0x8000,
                rom.slug_size_);
  }
};
