#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vm/vm.h"

void display_show(
    size_t x, size_t y,
    uint8_t framebuffer[VM_FRAMEBUF_WIDTH_BYTES * VM_FRAMEBUF_HEIGHT_BYTES]);
