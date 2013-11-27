#include <defs.h>

int getpid(){
  uint64_t pid;
 __asm__ __volatile__("movq $0x14, %rax\n\tint $0x80");
 __asm__ __volatile__("movq %%rax, %0;"
      :"=r"(pid)
        ://No input operands
      :"%rax"
     );
  return pid;
}
