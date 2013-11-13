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
.global _isr13
.global _isr14
.global _isr80
.extern fault_handler 
.extern sys_exit
.extern Schedule

#  0: Divide By Zero Exception
_isr0:
    cli
        pushq  $0x0   
        pushq $0x0
        jmp isr_common_stub

#  1: Debug Exception
_isr1:
   cli
        pushq $0x0
        pushq $0x1
        jmp isr_common_stub

#  13: General Protection Fault
_isr13:
    cli
        pushq $0x0
        pushq $0xD
        jmp isr_common_stub

#  14: Page fault exception
_isr14:
   cli
        pushq $0x0
        pushq $0xE
        jmp isr_common_stub

#  80: Software Interrupt
_isr80:
  cli
        cmpq $0x1, %rax
        jne .normal2
        callq sys_exit
        callq Schedule
        # Code should never return here
        .normal2:
        pushq $0x0
        pushq $0x80
        jmp isr_common_stub


isr_common_stub:
    PUSHAQ
    movq %rsp, %rdi    # Push us the stack
    callq fault_handler       # A special call, preserves the 'eip' register
    cmpq $0x80, 0x78(%rsp)
    jne .normal
    movq %rax, 0x70(%rsp)
    .normal:
    POPAQ
    add $0x10,%rsp     # Cleans up the pushed error code and pushed ISR number
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
