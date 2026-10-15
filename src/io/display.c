#include "io/display.h"

#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "vm/vm.h"

static void move_cursor(size_t x, size_t y) {
  printf("\033[%lu;%luH", y, x);
}

static void enter_raw_mode(struct termios* orig_termios) {
  struct termios new_termios;
  tcgetattr(STDIN_FILENO, orig_termios);
  new_termios = *orig_termios;

  // Disable canonical mode (line buffering) and local echo
  new_termios.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

  // Make standard input non-blocking
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

static void restore_mode(struct termios* orig_termios) {
  tcsetattr(STDIN_FILENO, TCSANOW, orig_termios);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
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

struct termios orig_termios;

void display_init() {
  // Switch to alternate screen.
  printf("\033[?1049h");

  enter_raw_mode(&orig_termios);

  // Hide cursor.
  printf("\033[?25l");

  // Clear the screen.
  printf("\e[1;1H\e[2J");
}

void display_exit() {
  // Go back to regular screen.
  printf("\033[?1049l");

  restore_mode(&orig_termios);

  // Restore cursor.
  printf("\033[?25h");
}

void display_play_sound() {
  putchar('\a');
  fflush(stdout);
}

void display_update(size_t display_x, size_t display_y,
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
