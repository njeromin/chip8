#include <stdio.h>
#include <stdlib.h>

#include "io/display.h"
#include "platform/thread.h"
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

  enum vm_exec_res exec_res;
  while ((exec_res = vm_exec_next(&vm)) == VM_EXEC_RES_CONTINUE) {
    display_show(0, 0, vm.framebuffer);
    platform_sleep(1000 / 60);
  }

  switch (exec_res) {
    case VM_EXEC_RES_COMPLETE:
      break;
    case VM_EXEC_RES_INVALID_INSTR:
      fprintf(stderr, "Encountered invalid instruction.");
      break;
    default:
      fprintf(stderr, "Unexpected execution result encountered.");
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
