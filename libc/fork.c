#include <defs.h>

int fork(){
  uint64_t pid;
  __asm__("movq $0x3, %rax;\n\tint $0x80");
   __asm__ __volatile__("movq %%rax, %0;"
      :"=r"(pid)
        ://No input operands
      :"%rax"
     );
  return pid;
}

