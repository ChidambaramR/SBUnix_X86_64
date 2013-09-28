.align 8

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

.align 8

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
.global _isr0
.global _isr1
.extern fault_handler 

#  0: Divide By Zero Exception
_isr0:
    cli
        pushq  $0x0   
        # A normal ISR stub that pops a dummy error coDe to keep a
                       # uniform stack frame
        pushq $0x0
        jmp isr_common_stub

#  1: Debug Exception
_isr1:
   cli
        pushq 0x0
        pushq 0x1
        jmp isr_common_stub

isr_common_stub:
    PUSHAQ
    movq %rsp, %rdi    # Push us the stack
    callq fault_handler       # A special call, preserves the 'eip' register
    POPAQ
    add $0x10,%rsp     # Cleans up the pushed error code and pushed ISR number
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
