#include "rom.h"

#include <arpa/inet.h>

#include <fstream>
#include <iostream>

Rom::Rom(std::shared_ptr<byte_t[]> contents)
    : contents_(contents),
      slug_SLUG_(this->read32(SLUGAddressSpecifier::SLUG)),
      slug_setup_(this->read32(
          SLUGAddressSpecifier::SETUP)),  // 0x01e0 is is the setup address
      slug_loop_(this->read32(SLUGAddressSpecifier::LOOP)),
      slug_load_data_address_(this->read32(SLUGAddressSpecifier::LDA_ROM)),
      slug_program_data_address_(this->read32(SLUGAddressSpecifier::PDA_RAM)),
      slug_size_(this->read32(SLUGAddressSpecifier::DATA_SIZE)) {}

Rom Rom::ReadRomFile(const std::string& filename) {
  // allocate memmory where the slug file contents will be stored
  std::shared_ptr<byte_t[]> slug_contents(new byte_t[SLUGValues::FILE_SIZE]);

  const std::string& extension = ".slug";
  bool valid_extension = ((filename.compare(filename.size() - extension.size(),
                                            extension.size(), extension) == 0));
  if (!valid_extension) {
    std::cerr << "Unsupported file extension." << std::endl;
    exit(1);
  }

  // Open the file
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  // Check if file is opened successfully
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    exit(1);
  }

  // check the size of the file to make sure it's correct
  if (file.tellg() != SLUGValues::FILE_SIZE) {
    std::cerr << "ERROR: SLUG file is the wrong size" << std::endl;
  }

  // Reset file pointer to the beginning
  file.seekg(0, std::ios::beg);

  // read in file contents
  char byte1;
  uint32_t in = 0;
  while (file.read(&byte1, 1)) {
    slug_contents[in++] = (byte_t)byte1;
  }

  // create the rom
  Rom slug_rom(slug_contents);

  // char byte1, byte2;
  // uint32_t in = 0;
  // while (file.read(&byte1, 1) && file.read(&byte2, 1)) {
  //   address_t bigEndian = (byte1 << 8 | byte2);
  //   address_t littleEndian = (bigEndian >> 8) | (bigEndian << 8);
  //   slug_contents[in++] = littleEndian;
  // }

  // Close the file
  file.close();

  return slug_rom;
}
