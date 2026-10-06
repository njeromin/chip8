#include "vm.h"

#include <string.h>

// Set all registers values to 0.
static void init_registers(struct vm_reg* registers) {
  memset(registers->v, 0, sizeof(*registers->v) * VM_REG_V_COUNT);
  registers->dt = 0;
  registers->st = 0;
  registers->i = 0;
  registers->pc = 0;
  registers->sp = 0;
}

void vm_ctx_init(struct vm_ctx* ctx) {
  init_registers(&ctx->registers);
  memset(ctx->stack, 0, sizeof(*ctx->stack) * VM_STACK_SIZE);
  memset(ctx->memory, 0, sizeof(*ctx->memory) * VM_MEMORY_SIZE);
  memset(ctx->framebuffer, 0,
         VM_FRAMEBUF_WIDTH_BYTES * VM_FRAMEBUF_HEIGHT_BYTES);
}

enum vm_load_err vm_load_bin(struct vm_ctx* ctx, uint8_t* bin, size_t size) {
  if (size > (VM_MEMORY_SIZE - VM_PROG_LOAD_ADDR)) {
    return VM_LOAD_ERR_EXCEEDS_MEMORY;
  }

  memcpy(ctx->memory + VM_PROG_LOAD_ADDR, bin, size);

  return VM_LOAD_ERR_OK;
}

enum vm_exec_res vm_exec_next(struct vm_ctx* ctx) {
  return VM_EXEC_RES_COMPLETE;
}
