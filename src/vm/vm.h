#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Memory size in bytes.
#define VM_MEMORY_SIZE 4096
// Stack size in bytes.
#define VM_STACK_SIZE 64

// Amount of V registers.
#define VM_REG_V_COUNT 16

#define VM_FONT_LOAD_ADDR 0x0
#define VM_FONT_MAX_ADDR 0x80
#define VM_PROG_LOAD_ADDR 0x200

#define VM_MAX_PROG_SIZE (VM_MEMORY_SIZE - VM_PROG_LOAD_ADDR)

// Framebuffer width in bits.
#define VM_FRAMEBUF_WIDTH 64
// Framebuffer height in bits.
#define VM_FRAMEBUF_HEIGHT 32
#define VM_FRAMEBUF_WIDTH_BYTES (VM_FRAMEBUF_WIDTH / 8)
#define VM_FRAMEBUF_SIZE_BYTES (VM_FRAMEBUF_WIDTH_BYTES * VM_FRAMEBUF_HEIGHT)

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
  uint16_t stack[VM_STACK_SIZE];
  uint8_t memory[VM_MEMORY_SIZE];
  uint8_t framebuffer[VM_FRAMEBUF_SIZE_BYTES];
  bool should_draw;
};

void vm_ctx_init(struct vm_ctx* ctx);

enum vm_load_err {
  VM_LOAD_ERR_OK = 0,
  VM_LOAD_ERR_EXCEEDS_MEMORY = -1,
};

enum vm_load_err vm_load_bin(struct vm_ctx* ctx, uint8_t* bin, size_t size);

enum vm_exec_res {
  VM_EXEC_RES_INVALID_INSTR = -1,
  VM_EXEC_RES_CONTINUE = 0,
  VM_EXEC_RES_COMPLETE = 1,
  VM_EXEC_RES_HALTED = 2,
};

enum vm_exec_res vm_exec_next(struct vm_ctx* ctx);
