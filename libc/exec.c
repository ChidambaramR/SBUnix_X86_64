#include <ustdlib.h>

int execve(const char* buf){
  int res;
  __asm__("movq $0x7, %rax;\n\tint $0x80;");
 __asm__ __volatile__("movl %%eax, %0;"
      :"=r"(res)
        ://No input operands
      :"%rax"
     );
  return res;
 
}

