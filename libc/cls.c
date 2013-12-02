#include <defs.h>
void cls(){
  __asm__("movq $0x9, %rax;\n\tint $0x80;");
  return ;
}

