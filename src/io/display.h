#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vm/vm.h"

void display_init();

void display_show(size_t x, size_t y,
                  uint8_t framebuffer[VM_FRAMEBUF_SIZE_BYTES]);
