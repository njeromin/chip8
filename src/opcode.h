#pragma once

#include <stdint.h>

typedef uint16_t opcode_t;

// Convert a big endian raw opcode into a host opcode.
opcode_t opcode_from_u8s(uint8_t left, uint8_t right);

// The i*** portion.
uint8_t opcode_id(opcode_t instr);
// The *NNN portion.
uint16_t opcode_nnn(opcode_t instr);
// The **NN portion.
uint8_t opcode_nn(opcode_t instr);
// The ***N portion.
uint8_t opcode_n(opcode_t instr);
// The *X** portion.
uint8_t opcode_x(opcode_t instr);
// The **Y* portion.
uint8_t opcode_y(opcode_t instr);
