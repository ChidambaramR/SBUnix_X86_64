#include<defs.h>
#include<stdio.h>

bool get_paging_status() {
  uint64_t cr0;

__asm__("movq %%cr0, %0;"
      :"=r"(cr0)
        ://No input operands
      :"%eax"
     );
  return (cr0 & 0x80000000) ? TRUE : FALSE;
}

uint64_t get_cr0_register(){
  uint64_t cr0;
__asm__("movq %%cr0, %0;"
      :"=r"(cr0)
        ://No input operands
      :"%eax"
     );
  return cr0;
}


uint64_t get_cr4_register(){
  uint64_t cr4;
__asm__("movq %%cr4, %0;"
      :"=r"(cr4)
        ://No input operands
      :"%eax"
     );
  return cr4;
}

uint64_t get_cr3_register(){
  uint64_t cr3;
__asm__("movq %%cr3, %0;"
      :"=r"(cr3)
        ://No input operands
      :"%eax"
     );
  return cr3;
}
void pmmngr_load_PDBR (uint64_t* addr) {
/*  uint64_t* val = addr; 
  asm volatile("movq %1, %%rax"
          "movq %%rax, %%cr3"
          :
          :"r"(val)
          :"%rax"
         );*/
}
