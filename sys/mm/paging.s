.text

.global vmmgr_load_pml4

vmmgr_load_pml4:
  movq %rdi, %rax
  movq %rax, %cr3
  retq
  

