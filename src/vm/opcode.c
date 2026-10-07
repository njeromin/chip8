#include "vm/opcode.h"

#include <endian.h>

opcode_t opcode_from_u8s(uint8_t left, uint8_t right) {
  return ((opcode_t)left << 8) | right;
}

uint8_t opcode_id(opcode_t instr) {
  return (instr & 0xF000) >> 12;
}

uint16_t opcode_nnn(opcode_t instr) {
  return (instr & 0x0FFF);
}

uint8_t opcode_nn(opcode_t instr) {
  return (instr & 0x00FF);
}

uint8_t opcode_n(opcode_t instr) {
  return (instr & 0x000F);
}

uint8_t opcode_x(opcode_t instr) {
  return (instr & 0x0F00) >> 8;
}

uint8_t opcode_y(opcode_t instr) {
  return (instr & 0x00F0) >> 4;
}
