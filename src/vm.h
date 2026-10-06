#pragma once

#include <stdint.h>

#define VM_MEMORY_SIZE 4096
#define VM_STACK_SIZE 64
#define VM_REG_V_COUNT 5

struct vm_reg {
  uint8_t v[VM_REG_V_COUNT];  // V registers
  uint8_t dt;                 // Delay timer
  uint8_t st;                 // Sound timer
  uint16_t i;                 // Index register
  uint16_t pc;                // Program counter
  uint8_t sp;                 // Stack pointer
};

struct vm_ctx {
  struct vm_reg registers;
  uint8_t stack[VM_STACK_SIZE];
  uint8_t memory[VM_MEMORY_SIZE];
};

void vm_ctx_init(struct vm_ctx* ctx);
