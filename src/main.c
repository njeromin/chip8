#include <stdlib.h>

#include "vm.h"

int main(int argc, char *argv[]) {
  struct vm_ctx vm = {};
  vm_ctx_init(&vm);

  return EXIT_SUCCESS;
}
