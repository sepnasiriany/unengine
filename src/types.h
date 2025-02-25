#pragma once

#include <arpa/inet.h>

#include <cstdint>
#include <functional>

using instruction_t = std::uint32_t;
using slug_space_t = std::uint32_t;
using register_value_t = std::uint16_t;
// 5 bytes for 32 registers
using register_index_t = std::uint8_t;
// 6 bit opcode
using opcode_t = std::uint8_t;
// 12 bit immediate
using immediate_t = std::uint16_t;

// 16 bit address
using address_t = register_value_t;

using byte_t = std::uint8_t;

// a word is 16 bits
using word_t = std::uint16_t;

using stopper_fn_t = std::function<void(void)>;

#ifdef RELEASE
#define warn(msg)
#else
#include <iostream>
#define warn(msg) std::cerr << (msg) << std::endl;
#endif
