#include <defs.h>
void print_pwd(){
  __asm__("movq $0x16, %rax;\n\tint $0x80;");
  return ;
}

