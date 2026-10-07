#include "io/display.h"

#include <stdio.h>
#include "vm/vm.h"

static void move_cursor(size_t x, size_t y) {
  printf("\033[%lu;%luH", y, x);
}

#define FULL_BOX "█"
#define UPPER_HALF_BOX "▀"
#define LOWER_HALF_BOX "▄"
#define EMPTY_BOX " "

static void calc_bit_pos(size_t x, size_t y,
                         uint8_t framebuffer[VM_FRAMEBUF_SIZE_BYTES],
                         size_t* byte_index, size_t* bit_offset) {
  *byte_index = (y * VM_FRAMEBUF_WIDTH + x) / 8;
  *bit_offset = 7 - (x % 8);
}

void display_init() {
  // Clear the screen.
  printf("\e[1;1H\e[2J");
}

void display_show(size_t display_x, size_t display_y,
                  uint8_t framebuffer[VM_FRAMEBUF_SIZE_BYTES]) {
  move_cursor(display_x, display_y);

  for (size_t y = 0; y + 1 < VM_FRAMEBUF_HEIGHT; y += 2) {
    for (size_t x = 0; x < VM_FRAMEBUF_WIDTH; x++) {
      size_t byte_index_upper, bit_offset_upper;
      calc_bit_pos(x, y, framebuffer, &byte_index_upper, &bit_offset_upper);

      size_t byte_index_lower, bit_offset_lower;
      calc_bit_pos(x, y + 1, framebuffer, &byte_index_lower, &bit_offset_lower);

      uint8_t bit_upper =
          (framebuffer[byte_index_upper] >> bit_offset_upper) & 1 ? 1 : 0;
      uint8_t bit_lower =
          (framebuffer[byte_index_lower] >> bit_offset_lower) & 1 ? 2 : 0;

      // Compare upper and lower flags.
      // 0 = neither
      // 1 = only upper
      // 2 = only lower
      // 3 = both
      switch (bit_upper + bit_lower) {
        case 0:
          printf(EMPTY_BOX);
          break;
        case 1:
          printf(UPPER_HALF_BOX);
          break;
        case 2:
          printf(LOWER_HALF_BOX);
          break;
        case 3:
          printf(FULL_BOX);
      }
    }

    printf("\n");
  }
}
