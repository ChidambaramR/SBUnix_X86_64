#include <defs.h>

void sleep(uint64_t time){
 __asm__ __volatile__("movq $0x4, %rax\n\tint $0x80");
}

