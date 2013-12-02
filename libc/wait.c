#include <defs.h>

int wait(){
  __asm__("movq $0x6, %rax;\n\tint $0x80;");
  return 0;
}

