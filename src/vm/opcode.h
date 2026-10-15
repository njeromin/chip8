#pragma once

#include <stdint.h>

typedef uint16_t opcode_t;

// Convert a big endian raw opcode into a host opcode.
opcode_t opcode_from_u8s(uint8_t left, uint8_t right);

// Read the id of an opcode (first 4 bits).
// - 0x(ID)000
uint8_t opcode_id(opcode_t instr);
// Read the NNN section of an opcode (last 12 bits).
// - 0x0NNN
uint16_t opcode_nnn(opcode_t instr);
// Read the NN section of an opcode (last 8 bits).
// - 0x00NN
uint8_t opcode_nn(opcode_t instr);
// Read the N section of an opcode (last 4 bits).
// - 0x000F
uint8_t opcode_n(opcode_t instr);
// Read the X section of an opcode (second set of 4 bits).
// - 0x0F00
uint8_t opcode_x(opcode_t instr);
// Read the Y section of an opcode (second-last set of 4 bits).
// - 0x00F0
uint8_t opcode_y(opcode_t instr);
