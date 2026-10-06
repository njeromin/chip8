#include "vm.h"

#include <string.h>

void init_registers(struct vm_reg* registers) {
  memset(registers->v, 0, sizeof(*registers->v) * VM_REG_V_COUNT);
  registers->dt = 0;
  registers->st = 0;
  registers->i = 0;
  registers->pc = 0;
  registers->sp = 0;
}

void vm_ctx_create(struct vm_ctx* ctx) {
  init_registers(&ctx->registers);
  memset(ctx->stack, 0, sizeof(*ctx->stack) * VM_STACK_SIZE);
  memset(ctx->memory, 0, sizeof(*ctx->memory) * VM_MEMORY_SIZE);
}
