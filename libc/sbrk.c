#include <defs.h>

int sbrk(int size){
 int pid;
 __asm__ __volatile__("movq $0xB, %rax\n\tint $0x80");
 __asm__ __volatile__("movl %%eax, %0;"
      :"=r"(pid)
        ://No input operands
      :"%rax"
     );
  return pid;
}

