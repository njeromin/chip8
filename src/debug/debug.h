#pragma once

#include <stdint.h>
#include <stdio.h>
#include <vm/vm.h>

// Returns the class of an instruction based off of its ID.
//
// For example,
// The instruction ID of 0xD000, 'D', would return "0xDXYN".
const char* debug_instruction_id_class(uint8_t instruction_id);

// Print information about the virtual machine.
//
// Prints:
// - Stack pointer value.
// - Addresses stored in the stack.
// - Registers values.
void debug_print_vm_info(FILE* restrict stream, const struct vm_ctx* ctx);
