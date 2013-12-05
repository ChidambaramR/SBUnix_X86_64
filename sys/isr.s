.extern currentThread
.extern tss
.extern is_scheduler_on
.extern runnable_kthread
.extern next_runnable_kthread
.extern do_execve
.extern Yield
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
        addq $0x8, %rsp
        pushq $0x0
        pushq $0xE
        jmp isr_common_stub

#  80: Software Interrupt
_isr80:
  cli
        #cmpq $0x7, %rax
        #jne .exit_t
        #callq do_execve
        #callq sys_exit
        #callq Schedule
#        callq Yield

        .exit_t:
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
    
    movq %rax, 0x70(%rsp)

    cmp $0x1, (is_scheduler_on)
    jne .normal


    movq (currentThread), %rax
    cmpq $0x0, 0x8(%rax)
    je .user_move_rsp_isr # The current thread is a user thread

    # This action is, if the current thread is a kernel thread
    # Its enough if we just move the stack pointer into the "rsp" field of kthread
    #movq (currentThread), %rax
    #movq %rsp, (%rax)
    lea (%rsp), %rax
    movq (currentThread), %rdi
    movq %rax, (%rdi)
    jmp .move_rsp_normal_isr

    # The current thread is a iser thread. We need to work with the kernel stack
    # We cant write in the user stack because on an interrupt, the RSP in the TSS gets loaded
    # So I create a kernel stack for each user and do push pop on the kernel's stack only.
    # The user's stack is untouched
    .user_move_rsp_isr:
    #movq (currentThread), %rax
    #movq %rsp, 0x10(%rax)
    lea (%rsp), %rax
    movq (currentThread), %rdi
    movq %rax, 0x10(%rdi)

    # usual operation
    .move_rsp_normal_isr:
    # Make the current thread runnable. i.e add it to the run queue
    movq (currentThread), %rdi
    callq runnable_kthread
    # Get the next thread which is to tbe scheduled. WARNING! It can be the same thread
    #   The address of the runnable thread is returned in the RAX register.
    callq next_runnable_kthread
    cmpq $0x0, 0x8(%rax)
    jne .store_tss_norm1_isr
    leaq (tss), %rbx
    leaq 0x4(%rbx), %rbx
    movq 0x28(%rax), %rcx
    movq %rcx, (%rbx)
    
    # !!!!!!!!!!!!!!!!!!!!!!!
    # THE MAIN CONTEXT SWITCH
    # !!!!!!!!!!!!!!!!!!!!!!!
    .store_tss_norm1_isr:
    movq %rax, (currentThread)
    # Resotre the the value of RSP which was stored before context switch occured
    # Again, we need comparison here
    movq (currentThread), %rbx
    movq %cr3, %rcx
    cmpq %rcx, 0x18(%rbx)
    je .norm_cr3_isr
    movq 0x18(%rbx), %rcx
    movq %rcx, %cr3

    .norm_cr3_isr:
    cmp $0x0, 0x8(%rax)
    je .user_change_rsp # The chosen thread is a user thread

    # if it comes here, then the chosen thread is a kernel thread. So stack pointer is
    #   obtained from the "rsp" field. i.e the 1st member in the kthread
    movq (%rax), %rsp
    jmp .normal_change_rsp_isr

    # The chosen thread is a user thread
    .user_change_rsp:
    # We restore the stack pointer from the "krsp" field of the kthread
    movq 0x10(%rax), %rsp

    # normal operation. So, here ends the context switch. Phew!!!
    .normal_change_rsp_isr:
    POPAQ
    add $0x10, %rsp
    iretq

    .normal:
    POPAQ
    add $0x10,%rsp     # Cleans up the pushed error code and pushed ISR number
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
