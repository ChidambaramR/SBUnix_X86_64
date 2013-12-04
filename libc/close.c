#include <defs.h>
void close(int fd){
  __asm__("movq $0xD, %rax;\n\tint $0x80;");
  return;
}

