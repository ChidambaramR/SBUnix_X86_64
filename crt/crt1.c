#include <stdlib.h>


void _start(void) {
	int argc = 1;
	char* argv[10];
	char* envp[10];
	int res;
	res = main(argc, argv, envp);
	exit(res);
}

void exit(res){
      __asm__ __volatile__("movq $0x14, %rax\n\tint $0x80");
      __asm__ __volatile__("movq %rax, %rdi\n\tmovq $0x1, %rax\n\tint $0x80");
}
