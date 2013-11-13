.extern currentThread
.extern is_scheduler_on
.extern runnable_kthread
.extern next_runnable_kthread

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
.global _irq0
.global _irq1
.global _irq2
.global _irq3
.global _irq4
.global _irq5
.global _irq6
.global _irq7
.global _irq8
.global _irq9
.global _irq10
.global _irq11
.global _irq12
.global _irq13
.global _irq14
.global _irq15

.extern fault_handler 

#  IRQ 0: IDT 32 
_irq0:
    cli
        pushq  $0x0 # error code   
        pushq  $0x32 # idt no
        jmp irq_common_stub


#  IRQ 1: IDT 33
_irq1:
    cli
        pushq  $0x0 # error code   
        pushq  $0x33 # idt no
        jmp irq_common_stub

#  IRQ 2: IDT 34
_irq2:
    cli
        pushq  $0x0 # error code   
        pushq  $0x34 # idt no
        jmp irq_common_stub


#  IRQ 3: IDT 35
_irq3:
    cli
        pushq  $0x0 # error code   
        pushq  $0x35 # idt no
        jmp irq_common_stub


#  IRQ 4: IDT 36
_irq4:
    cli
        pushq  $0x0 # error code   
        pushq  $0x36 # idt no
        jmp irq_common_stub

#  IRQ 5: IDT 37
_irq5:
    cli
        pushq  $0x0 # error code   
        pushq  $0x37 # idt no
        jmp irq_common_stub

#  IRQ 6: IDT 38 
_irq6:
    cli
        pushq  $0x0 # error code   
        pushq  $0x38 # idt no
        jmp irq_common_stub

#  IRQ 7: IDT 39 
_irq7:
    cli
        pushq  $0x0 # error code   
        pushq  $0x39 # idt no
        jmp irq_common_stub

#  IRQ 8: IDT 40 
_irq8:
    cli
        pushq  $0x0 # error code   
        pushq  $0x40 # idt no
        jmp irq_common_stub

#  IRQ 9: IDT 41 
_irq9:
    cli
        pushq  $0x0 # error code   
        pushq  $0x41 # idt no
        jmp irq_common_stub

#  IRQ 10: IDT 42 
_irq10:
    cli
        pushq  $0x0 # error code   
        pushq  $0x42 # idt no
        jmp irq_common_stub

#  IRQ 11: IDT 43 
_irq11:
    cli
        pushq  $0x0 # error code   
        pushq  $0x43 # idt no
        jmp irq_common_stub

#  IRQ 12: IDT 44 
_irq12:
    cli
        pushq  $0x0 # error code   
        pushq  $0x44 # idt no
        jmp irq_common_stub

#  IRQ 13: IDT 45 
_irq13:
    cli
        pushq  $0x0 # error code   
        pushq  $0x45 # idt no
        jmp irq_common_stub

#  IRQ 14: IDT 46
_irq14:
    cli
        pushq  $0x0 # error code   
        pushq  $0x46 # idt no
        jmp irq_common_stub

#  IRQ 15 : IDT 47: 
_irq15:
    cli
        pushq  $0x0 # error code   
        pushq  $0x36 # irq no
        jmp irq_common_stub

.extern irq_handler

irq_common_stub:
    PUSHAQ
    movq %rsp, %rdi    # Push us the stack
    callq irq_handler
    # Getting the interrupt number we pushed. Offset is determinged manually. 
    movq 0x78(%rsp), %rax
    # Handle context switching for a timer interrupt. 
    cmp $0x32, %rax
    jne .restore_reg
    # If scheduler is not initialized then we should not context switch.
    #   is_schedule_on is turned on by scheduler_init
    cmp $0x1, (is_scheduler_on)
    jne .restore_reg
    lea (%rsp), %rax
    movq (currentThread), %rdi
    movq %rax, (%rdi)
    # Make the current thread runnable. i.e add it to the run queue
    callq runnable_kthread
    # Get the next thread which is to tbe scheduled. WARNING! It can be the same thread
    #   The address of the runnable thread is returned in the RAX register.
    callq next_runnable_kthread
    # !!!!!!!!!!!!!!!!!!!!!!!
    # THE MAIN CONTEXT SWITCH
    # !!!!!!!!!!!!!!!!!!!!!!!
    movq %rax, (currentThread)
    # Resotre the the value of RSP which was stored before context switch occured
    movq (%rax), %rsp
    POPAQ
    # Cleaning the interrupt number and error code we pushed during _irq0 call.
    add $0x10, %rsp
    # cmpq $0x1, 0x8(currentThread)
    # jne .user_mode
    # movq $0x8, 0x8(%rsp)

    iretq
    
.restore_reg:
    POPAQ
    add $0x10, %rsp     # Cleans up the pushed error code and pushed ISR number
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
