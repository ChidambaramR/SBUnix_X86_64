
void fork(){
  __asm__("movq $0x3, %rax;\n\tint $0x80");
  return; 
}

