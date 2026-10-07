#include "vm/vm.h"

#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm/font.h"
#include "vm/opcode.h"

static void clear_display(struct vm_ctx* ctx) {
  memset(ctx->framebuffer, 0, VM_FRAMEBUF_SIZE_BYTES);
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

  // Load font sprites into memory.
  memcpy(ctx->memory + VM_FONT_LOAD_ADDR, CHIP8_FONTSET, sizeof(CHIP8_FONTSET));

  clear_display(ctx);

  ctx->should_draw = false;
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
            ctx->should_draw = true;
            break;
          case 0x0EE:
            // Return.
            ctx->registers.pc = ctx->stack[ctx->registers.sp];
            ctx->registers.sp -= 1;
            skip_pc_increment = true;
            break;
          case 0x000:
            // Nop
            return VM_EXEC_RES_INVALID_INSTR;
          default:
            // Call NNN.
            // NOTE: This does not need to implemented.
            return VM_EXEC_RES_INVALID_INSTR;
        }

        break;
      }
    case 0x1:
      {
        // Jump to address NNN.
        uint16_t nnn = opcode_nnn(code);

        // Detect self-jump spinloop (halt)
        if (nnn == ctx->registers.pc) {
          return VM_EXEC_RES_HALTED;
        }

        ctx->registers.pc = nnn;
        skip_pc_increment = true;

        break;
      }
    case 0x2:
      {
        // Call subroutine at NNN.
        ctx->registers.sp += 1;
        ctx->stack[ctx->registers.sp] = ctx->registers.pc + sizeof(opcode_t);
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
        uint8_t x = opcode_x(code);
        uint8_t y = opcode_y(code);

        switch (n) {
          case 0x0:
            // VX = VY
            ctx->registers.v[x] = ctx->registers.v[y];
            break;
          case 0x1:
            // VX |= VY
            ctx->registers.v[x] |= ctx->registers.v[y];
            break;
          case 0x2:
            // VX &= VY
            ctx->registers.v[x] &= ctx->registers.v[y];
            break;
          case 0x3:
            // VX ^= VY
            ctx->registers.v[x] ^= ctx->registers.v[y];
            break;
          case 0x4:
            {
              // VX += VY
              uint16_t res = ctx->registers.v[x] + ctx->registers.v[y];
              uint8_t carry = (res > 0xFF) ? 1 : 0;

              // VF = 0 when no overflow, 1 when overflow
              ctx->registers.v[0xF] = carry;
              ctx->registers.v[x] = (uint8_t)(res & 0xFF);

              break;
            }
          case 0x5:
            {
              // VX -= VY
              uint8_t not_borrow =
                  (ctx->registers.v[x] >= ctx->registers.v[y]) ? 1 : 0;
              uint8_t res = ctx->registers.v[x] - ctx->registers.v[y];

              // VF = 0 when underflow, 1 when no underflow.
              ctx->registers.v[0xF] = not_borrow;
              ctx->registers.v[x] = res;

              break;
            }
          case 0x6:
            {
              // VX >>= 1
              uint8_t lsb = ctx->registers.v[x] & 0x1;
              uint8_t res = ctx->registers.v[x] >> 1;

              // Store LSB of VX prior to shift.
              ctx->registers.v[0xF] = lsb;
              ctx->registers.v[x] = res;

              break;
            }
          case 0x7:
            {
              // VX = VY - VX
              uint8_t not_borrow =
                  (ctx->registers.v[y] >= ctx->registers.v[x]) ? 1 : 0;
              uint8_t res = ctx->registers.v[y] - ctx->registers.v[x];

              // VF = 0 when underflow, 1 when no underflow.
              ctx->registers.v[0xF] = not_borrow;
              ctx->registers.v[x] = res;

              break;
            }
          case 0xE:
            {
              // VX <<= 1
              uint8_t msb = (ctx->registers.v[x] >> 7) & 0x1;
              uint8_t res = ctx->registers.v[x] << 1;

              // Store MSB of VX prior to shift.
              ctx->registers.v[0xF] = msb;
              ctx->registers.v[x] = res;

              break;
            }
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

        uint8_t x = opcode_x(code);
        uint8_t y = opcode_y(code);

        if (ctx->registers.v[x] != ctx->registers.v[y]) {
          ctx->registers.pc += sizeof(opcode_t);
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
      {
        uint8_t x = opcode_x(code);
        uint8_t nn = opcode_nn(code);

        ctx->registers.v[x] = (rand() % 255) & nn;

        break;
      }
    case 0xD:
      {
        // Draw a sprite at the coordinate (VX, VY) that has a width of 8 pixels, and a height of N pixels.
        uint8_t x = opcode_x(code);
        uint8_t y = opcode_y(code);
        uint8_t n = opcode_n(code);  // height

        // Starting coordinates (wrapped to display boundaries)
        uint8_t start_x = ctx->registers.v[x] % VM_FRAMEBUF_WIDTH;
        uint8_t start_y = ctx->registers.v[y] % VM_FRAMEBUF_HEIGHT;

        // Reset collision flag
        ctx->registers.v[0xF] = 0;

        for (size_t row = 0; row < n; row++) {
          size_t y = start_y + row;
          // Clip sprite if it exceeds the bottom of the screen
          if (y >= VM_FRAMEBUF_HEIGHT) {
            break;
          }

          uint8_t sprite_byte = ctx->memory[ctx->registers.i + row];

          for (size_t col = 0; col < 8; col++) {
            size_t x = start_x + col;
            // Clip sprite if it exceeds the right edge of the screen
            if (x >= VM_FRAMEBUF_WIDTH) {
              break;
            }

            // Check if the current sprite pixel is 1 (MSB is col 0)
            uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 1;
            if (!sprite_pixel) {
              continue;
            }

            // Calculate byte index and bit offset in 1-bit framebuffer
            size_t byte_idx = (y * VM_FRAMEBUF_WIDTH + x) / 8;
            size_t bit_offset = 7 - (x % 8);

            // Check collision: screen pixel is currently 1 and will be unset
            if ((ctx->framebuffer[byte_idx] >> bit_offset) & 1) {
              ctx->registers.v[0xF] = 1;
            }

            // XOR pixel onto the screen
            ctx->framebuffer[byte_idx] ^= (1 << bit_offset);
          }
        }

        ctx->should_draw = true;

        break;
      }
    case 0xE:
      {
        uint8_t nn = opcode_nn(code);

        switch (nn) {
          case 0x9E:
            // Skip the next instruction if the key stored in VX is pressed.
            break;
          case 0xA1:
            // Skip the next instruction if the key stored in VX is not pressed.
            break;
        }

        break;
      }
    case 0xF:
      {
        uint8_t nn = opcode_nn(code);
        uint8_t x = opcode_x(code);

        switch (nn) {
          case 0x07:
            // Set VX to the value of the delay timer.
            ctx->registers.v[x] = ctx->registers.dt;
            break;
          case 0x0A:
            break;
          case 0x15:
            // Set the delay timer to the value in VX.
            ctx->registers.dt = ctx->registers.v[x];
            break;
          case 0x18:
            // Set the sound timer to the value in VX.
            ctx->registers.st = ctx->registers.v[x];
            break;
          case 0x1E:
            // Add VX to I.
            ctx->registers.i += ctx->registers.v[x];
            break;
          case 0x29:
            // Set I to the memory address of the sprite for digit in Vx (0x0 - 0xF).
            ctx->registers.i =
                VM_FONT_LOAD_ADDR +
                ((ctx->registers.v[x] & 0x0F) * VM_FONT_CHAR_HEIGHT);
            break;
          case 0x33:
            {
              // Store the BCD representation of VX in I (hundreds), I+1 (tens), I+2 (ones).
              uint8_t value = ctx->registers.v[x];
              uint16_t i_addr = ctx->registers.i;

              ctx->memory[i_addr] = value / 100;
              ctx->memory[i_addr + 1] = (value / 10) % 10;
              ctx->memory[i_addr + 2] = value % 10;

              break;
            }
          case 0x55:
            {
              // Store V0 to VX in memory, starting at address I.
              uint16_t i_addr = ctx->registers.i;

              for (uint16_t v_idx = 0; v_idx < VM_REG_V_COUNT; v_idx++) {
                uint8_t reg_value = ctx->registers.v[v_idx];
                ctx->memory[i_addr + v_idx] = reg_value;
              }

              break;
            }
          case 0x65:
            {
              // Fill V0 to VX with values from memory, starting at address I.
              uint16_t i_addr = ctx->registers.i;

              for (uint16_t v_idx = 0; v_idx < VM_REG_V_COUNT; v_idx++) {
                ctx->registers.v[v_idx] = ctx->memory[i_addr + v_idx];
              }

              break;
            }
        }

        break;
      }
  }

  if (!skip_pc_increment) {
    ctx->registers.pc += sizeof(opcode_t);
  }

  return VM_EXEC_RES_CONTINUE;
}
