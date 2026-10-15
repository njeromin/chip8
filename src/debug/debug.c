#include "debug/debug.h"

#include <stdio.h>
#include "vm/vm.h"

const char* debug_instruction_id_class(uint8_t instruction_id) {
  switch (instruction_id) {
    case 0x0:
      return "0x00NN";
    case 0x1:
      return "0x1NNN";
    case 0x2:
      return "0x2NNN";
    case 0x3:
      return "0x3XNN";
    case 0x4:
      return "0x4XNN";
    case 0x5:
      return "0x5XY0";
    case 0x6:
      return "0x6XNN";
    case 0x7:
      return "0x7XNN";
    case 0x8:
      return "0x8XYN";
    case 0x9:
      return "0x9XY0";
    case 0xA:
      return "0xANNN";
    case 0xB:
      return "0xBNNN";
    case 0xC:
      return "0xCXNN";
    case 0xD:
      return "0xDXYN";
    case 0xE:
      return "0xEXNN";
    case 0xF:
      return "0xFXNN";
    default:
      return "UNKNOWN";
  }
}

void debug_print_vm_info(FILE* restrict stream, const struct vm_ctx* ctx) {
  fprintf(stream, "SP: %u\n", ctx->registers.sp);
  fprintf(stream, "  #%u: 0x%03X (current PC)\n", ctx->registers.sp,
          ctx->registers.pc);
  for (size_t i = (size_t)ctx->registers.sp; i > 0; i--) {
    fprintf(stream, "  #%lu: 0x%03X\n", i - 1, ctx->stack[i]);
  }

  fprintf(stream, "\nRegisters:\n");

  for (size_t i = 0; i < VM_REG_V_COUNT; i++) {
    uint8_t value = ctx->registers.v[i];
    // NOTE: Only aligned properly for 0 to 99.
    fprintf(stream, "  V%lu: %c0x%08x\n", i, i >= 10 ? 0 : ' ', value);
  }
}
