#include <defs.h>
uint16_t ps(){
  __asm__("movq $0x8, %rax;\n\tint $0x80;");
  return 0;
}

