#pragma once

#include <memory>

#include "types.h"

// specific values important to slug files
enum SLUGValues {
  FILE_SIZE = 0x8000,
};

// const int SLUG_FILE_SIZE = (int)0x8000;
// const PermissionBits SLUG_PERMISSIONS = (PesrmissionBits)(Read | Execute);

// slug file content address distance from start of slug file (0x8000)
enum SLUGAddressSpecifier {
  SLUG = 0x0000,
  SETUP = 0x01e0,
  LOOP = 0x01e4,
  LDA_ROM = 0x01e8,
  PDA_RAM = 0x01ec,
  DATA_SIZE = 0x01f0,
};

// Contains all the data from a SLUG file
class Rom {
 private:
  std::shared_ptr<byte_t[]> contents_;
  Rom(std::shared_ptr<byte_t[]> contents);

 public:
  const slug_space_t slug_SLUG_;
  const slug_space_t slug_setup_;
  const slug_space_t slug_loop_;
  const slug_space_t slug_load_data_address_;
  const slug_space_t slug_program_data_address_;
  const slug_space_t slug_size_;

  const std::shared_ptr<byte_t[]> contents() const { return contents_; }

  std::uint32_t read32(address_t addr) const {
    return htonl(reinterpret_cast<const uint32_t*>(
        contents_.get())[addr / sizeof(uint32_t)]);
  }

  static Rom ReadRomFile(const std::string& filename);
};
