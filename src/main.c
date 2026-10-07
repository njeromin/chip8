#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug/debug.h"
#include "io/display.h"
#include "platform/thread.h"
#include "vm/opcode.h"
#include "vm/vm.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: chip8 <filename>\n");
    return EXIT_FAILURE;
  }

  struct vm_ctx vm = {};
  vm_ctx_init(&vm);

  FILE* file = fopen(argv[1], "rb");
  if (file == NULL) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  uint8_t buf[VM_MAX_PROG_SIZE];
  size_t bytes_read = fread(buf, 1, sizeof(buf), file);

  if (ferror(file)) {
    perror("Error reading file");
    fclose(file);
    return EXIT_FAILURE;
  }

  // Check if the file was larger than available VM memory.
  if (fgetc(file) != EOF) {
    fprintf(stderr, "Error: ROM exceeds available memory\n");
    fclose(file);
    return EXIT_FAILURE;
  }

  switch (vm_load_bin(&vm, buf, bytes_read)) {
    case VM_LOAD_ERR_EXCEEDS_MEMORY:
      fprintf(stderr, "Load binary error: exceeds memory\n");
      return EXIT_FAILURE;
    case VM_LOAD_ERR_OK:
      break;
  }

  display_init();

  enum vm_exec_res exec_res;
  while ((exec_res = vm_exec_next(&vm)) == VM_EXEC_RES_CONTINUE ||
         exec_res == VM_EXEC_RES_HALTED) {
    if (vm.should_draw) {
      display_show(0, 0, vm.framebuffer);
      vm.should_draw = false;
    }

    if (exec_res == VM_EXEC_RES_HALTED) {
      platform_sleep(1000);
    }
  }

  switch (exec_res) {
    case VM_EXEC_RES_COMPLETE:
    case VM_EXEC_RES_HALTED:
      break;
    case VM_EXEC_RES_INVALID_INSTR:
      {
        opcode_t op = opcode_from_u8s(vm.memory[vm.registers.pc],
                                      vm.memory[vm.registers.pc + 1]);
        uint8_t id = opcode_id(op);

        fprintf(stderr, "Encountered invalid instruction: %s\n",
                debug_instruction_id_class(id));
        break;
      }
    default:
      fprintf(stderr, "Unexpected execution result encountered.");
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
