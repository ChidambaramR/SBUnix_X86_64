.extern currentThread
.extern KERN_CS
.extern KERN_DS
.extern USER_CS
.extern USER_CS
.extern tss

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
.global Switch_To_Thread
.global get_flag_register
.extern currenThread

# This is a HACK. I am re-ordeing the stack to support a return from
# IRETQ instruction. The IRETQ expects the stack to be in the following fashion.
# After my hack the stack will be as follows.
  # KERN_DS
  # RSP
  # RFLAGS
  # KERN_CS
  # RIP___rsp
# Thus when an IRETQ instruction happens, it will pop RIP, KERN_CS, RFLAGS, RSP and KERN_CS
#   in the order which was mentioned.

Switch_To_Thread:
  pushq %rax
  leaq (currentThread), %rax
  movq (%rax), %rax
  cmpq $0x0, 0x8(%rax)
  je .user_ds
  popq %rax
  pushq KERN_DS
  jmp .resume
.user_ds:
  popq %rax
  pushq USER_DS
.resume:
  pushq %rsp
  pushq %rax
  # This gets the return address pushed by the CPU on receipt of an interrupt.
  # Why we are adding by 0x18, is that, the stack before the following move will be as follows. 
    # Return address pushed by the CPU
    # KERN_DS
    # RSP
    # RAX ___rsp. 
    #   Thus, adding 0x18 to RSP will move to the RSP to the start of the return address so that
    #   a move from that point will fetch the return address
  movq 0x18(%rsp), %rax
  # The following instruction moves the return address to the bottom of the stack.
  movq %rax, -0x10(%rsp)
  # Restoring the contents of RAX
  movq %rsp, %rax
  add $0x8, %rsp
  pushfq
  pushq %rax
  leaq (currentThread), %rax
  movq (%rax), %rax
  cmpq $0x0, 0x8(%rax)
  je .user_cs
  popq %rax
  pushq KERN_CS
  jmp .resume1
.user_cs:
  popq %rax
  pushq USER_CS
.resume1:
  sub $0x8, %rsp
  pushq $0x0
  pushq $0x0
  PUSHAQ
  cmpq $0x0, 0x8(%rdi)
  jne .store_tss_norm
  pushq %rbx
  leaq (tss), %rax
  leaq 0x4(%rax), %rax
  movq 0x28(%rdi), %rbx
  movq %rbx, (%rax)
  popq %rbx
 .store_tss_norm:

  # This is the stack capture before a context switch.
    # KERN_DS
    # RSP
    # RFLAGS
    # KERN_CS
    # RIP
    # 0
    # 0
    # All the 15 registers___rsp
  movq (currentThread), %rax
  # Saving the RSP int the rsp member of the current thread's kthread structure.
  movq %rsp, (%rax)

  # !!!!!!!!!!!!!!!!!!!!!!!!!
  # THE CONTEXT SWITCH!!!!!!!
  # !!!!!!!!!!!!!!!!!!!!!!!!
  movq %rdi , (currentThread)
  movq (currentThread), %rbx
  movq %cr3, %rcx
  cmpq %rcx, 0x18(%rbx)
  je .norm_cr31
  movq 0x18(%rbx), %rcx
  movq %rcx, %cr3

  .norm_cr31:
  # Restoring the new thread's RSP (i.e where it left off )
  movq (currentThread), %rbx
  cmp $0x1, 0x8(%rbx)
  je .kern_change_rsp_sched
  movq 0x10(%rdi), %rsp
  jmp .norm_move_rsp_sched

  .kern_change_rsp_sched:
  movq (%rdi), %rsp

  .norm_move_rsp_sched:
  POPAQ
  # Cleaning up the fake 0x0's we pushed ( both at setup_thread and before context switch.
  add $0x10, %rsp
  # !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  # Going back to where we left off
  # !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  iretq

get_flag_register:
  pushfq
  popq %rax
  retq

