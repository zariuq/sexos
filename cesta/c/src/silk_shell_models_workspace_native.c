#include "silk_shell_models_workspace.h"
uint64_t shell_models_workspace_info(uint64_t slot)
{
 uint64_t status=63,kind=0;
 __asm__ volatile("syscall" : "+a"(status),"+S"(kind) : "D"(slot)
  : "rdx","r10","r8","rcx","r11","memory","cc");
 return status;
}


