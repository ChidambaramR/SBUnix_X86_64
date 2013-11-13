.extern hello_in_user_mode

.macro PUSHAQ
   #Save registers to the stack.

   pushq %rax      #save current rax
   pushq %rbx      #save current rbx
   pushq %rcx      #save current rcx
   pushq %rdx      #save current rdx
   pushq %rbp      #save current rbp
   pushq %rdi      #save current rdi
   pushq %rsi      #save current rsi
   pushq %r8       #save current r8
   pushq %r9       #save current r9
   pushq %r10      #save current r10
   pushq %r11      #save current r11
   pushq %r12      #save current r12
   pushq %r13      #save current r13
   pushq %r14      #save current r14
   pushq %r15      #save current r15

.endm   #end of macro definition

.macro POPAQ
   #Restore registers from the stack.

   popq %r15         #restore current r15
   popq %r14         #restore current r14
   popq %r13         #restore current r13
   popq %r12         #restore current r12
   popq %r11         #restore current r11
   popq %r10         #restore current r10
   popq %r9          #restore current r9
   popq %r8          #restore current r8
   popq %rsi         #restore current rsi
   popq %rdi         #restore current rdi
   popq %rbp         #restore current rbp
   popq %rdx         #restore current rdx
   popq %rcx         #restore current rcx
   popq %rbx         #restore current rbx
   popq %rax         #restore current rax

.endm         #end of macro definition


.text
.global switch_to_user
.extern currentThread
.extern tss

switch_to_user:
cli
movq $0x23, %rax # USER DS = 0x20 | 0x3 ( DPL ) = 0x23
movq %rax, %ds
movq %rax, %es
movq %rax, %fs
movq %rax, %gs
leaq (tss), %rax
leaq 0x4(%rax), %rax
movq %rsp, (%rax)
movq (currentThread), %rax
pushq $0x23
pushq (%rax)
# pushq $0x900000
pushfq
popq %rax
orq $0x200, %rax
pushq %rax
pushq $0x1B # User CS = 0x18 | 0x3 ( DPL ) = 0x1B
movq (currentThread), %rax
leaq 0x8(%rax), %rax
movq (%rax), %rax
# leaq 0x8(currentTask), %rax
pushq %rax
# pushq $0x800000
iretq 

1:
