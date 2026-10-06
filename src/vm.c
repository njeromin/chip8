#include "vm.h"

#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "opcode.h"

static void clear_display(struct vm_ctx* ctx) {
  memset(ctx->framebuffer, 0,
         VM_FRAMEBUF_WIDTH_BYTES * VM_FRAMEBUF_HEIGHT_BYTES);
}

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
  clear_display(ctx);
}

enum vm_load_err vm_load_bin(struct vm_ctx* ctx, uint8_t* bin, size_t size) {
  if (size > (VM_MEMORY_SIZE - VM_PROG_LOAD_ADDR)) {
    return VM_LOAD_ERR_EXCEEDS_MEMORY;
  }

  memcpy(ctx->memory + VM_PROG_LOAD_ADDR, bin, size);

  // Move program counter to load address.
  ctx->registers.pc = VM_PROG_LOAD_ADDR;

  return VM_LOAD_ERR_OK;
}

enum vm_exec_res vm_exec_next(struct vm_ctx* ctx) {
  if (ctx->registers.pc >= VM_MEMORY_SIZE) {
    return VM_EXEC_RES_COMPLETE;
  }

  bool skip_pc_increment = false;

  uint16_t raw_code_l = ctx->memory[ctx->registers.pc];
  uint16_t raw_code_r = ctx->memory[ctx->registers.pc + 1];
  opcode_t code = opcode_from_u8s(raw_code_l, raw_code_r);

  uint8_t id = opcode_id(code);

  switch (id) {
    case 0x0:
      {
        uint16_t nnn = opcode_nnn(code);

        switch (nnn) {
          case 0x0E0:
            // Clear the screen.
            clear_display(ctx);
            break;
          case 0x0EE:
            // Return.
            ctx->registers.pc = ctx->stack[ctx->registers.sp];
            ctx->registers.sp -= 1;
            break;
          default:
            // Call NNN.
            // NOTE: This does not need to implemented.
            printf("Unexpected instruction call: 0nnn\n");
            break;
        }

        break;
      }
    case 0x1:
      {
        // Jump to address NNN.
        uint16_t nnn = opcode_nnn(code);
        ctx->registers.pc = nnn;
        skip_pc_increment = true;
        break;
      }
    case 0x2:
      {
        // Call subroutine at NNN.
        ctx->registers.sp += 1;
        ctx->stack[ctx->registers.sp] = ctx->registers.pc;
        ctx->registers.pc = opcode_nnn(code);
        skip_pc_increment = true;
        break;
      }
    case 0x3:
      {
        // Skip the next instruction if VX equals NN.
        uint8_t x = opcode_x(code);
        uint8_t nn = opcode_nn(code);

        if (ctx->registers.v[x] == nn) {
          ctx->registers.pc += sizeof(opcode_t) * 2;
          skip_pc_increment = true;
        };

        break;
      }
    case 0x4:
      {
        // Skip the next instruction if VX does not equals NN.
        uint8_t x = opcode_x(code);
        uint8_t nn = opcode_nn(code);

        if (ctx->registers.v[x] != nn) {
          ctx->registers.pc += sizeof(opcode_t) * 2;
          skip_pc_increment = true;
        };

        break;
      }
    case 0x5:
      {
        // Skip the next instruction if VX equals VY.
        uint8_t x = opcode_x(code);
        uint8_t y = opcode_y(code);

        if (ctx->registers.v[x] == ctx->registers.v[y]) {
          ctx->registers.pc += sizeof(opcode_t);
        }

        break;
      }
    case 0x6:
      {
        // Sets VX to NN.
        uint8_t x = opcode_x(code);
        uint8_t nn = opcode_nn(code);

        ctx->registers.v[x] = nn;

        break;
      }
    case 0x7:
      {
        // Adds NN to VX.
        uint8_t x = opcode_x(code);
        uint8_t nn = opcode_nn(code);

        ctx->registers.v[x] += nn;

        break;
      }
    case 0x8:
      {
        uint8_t n = opcode_n(code);

        switch (n) {
          case 0x0:
            break;
          case 0x1:
            break;
          case 0x2:
            break;
          case 0x3:
            break;
          case 0x4:
            break;
          case 0x5:
            break;
          case 0x6:
            break;
          case 0x7:
            break;
          case 0xE:
            break;
        }

        break;
      }
    case 0x9:
      {
        // Skips the next instruction if VX does not equal VY.
        uint8_t n = opcode_n(code);

        if (n != 0) {
          return VM_EXEC_RES_INVALID_INSTR;
        }

        break;
      }
    case 0xA:
      {
        // Sets I to the address NNN.
        uint16_t nnn = opcode_nnn(code);
        ctx->registers.i = nnn;

        break;
      }
    case 0xB:
      {
        // Jumps to the address NNN plus V0.
        uint16_t nnn = opcode_nnn(code);
        ctx->registers.pc = nnn + ctx->registers.v[0];
        skip_pc_increment = true;

        break;
      }
    case 0xC:
      break;
    case 0xD:
      break;
    case 0xE:
      break;
    case 0xF:
      break;
  }

  if (!skip_pc_increment) {
    ctx->registers.pc += sizeof(opcode_t);
  }

  return VM_EXEC_RES_CONTINUE;
}
