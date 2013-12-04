#include <ustdlib.h>
#include <stdarg.h>
#include <defs.h>

int cd(char* buf){
  int ret;
  __asm__("movq $0x15, %rax;\n\tint $0x80;");
 __asm__ __volatile__("movl %%eax, %0;"
      :"=r"(ret)
        ://No input operands
      :"%rax"
     );
  return ret;
}

