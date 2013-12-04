#include <defs.h>
uint16_t ls(){
  __asm__("movq $0xA, %rax;\n\tint $0x80;");
  return 0;
}

