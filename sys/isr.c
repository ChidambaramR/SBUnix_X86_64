#include <defs.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/kthread.h>
extern kthread* currentThread;
extern kthread* ptable[100];
void page_fault_handler(uint64_t err_code, void* err_ins){
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint64_t faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   // The error code gives us details of what happened.
   int present = !(err_code & 0x1); // Page not present
   int rw = err_code & 0x2;           // Write operation?
   int us = err_code & 0x4;           // Processor was in user-mode?
   int reserved = err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
//   int id = err_code & 0x10;          // Caused by an instruction fetch?

   // Output an error message.
//   cls();
   printf("Page fault! ( ");
   if (present) {printf("Page Not Present ");}
   if (rw) {printf("read-only ");}
   if (us) {printf("user-mode ");}
   if (reserved) {printf("reserved ");}
   printf(") at 0x");
   printf("%p",faulting_address);
   printf("\n");
   printf("Faulting ins %p\n",((uint64_t)(err_ins)));
   PANIC(__FUNCTION__,__LINE__,"Page fault! ");
   while(1);
}

void general_protection_fault_handler(uint64_t err_code){
//  cls();
  PANIC(__FUNCTION__,__LINE__,"General Protection Fault! ");
}

void write(const char* str){
  printf("%s",str);
}

int sys_getpid(){
  return currentThread->pid;
}

void sys_exit(){
  int pid;
  kthread* k_thread;
  /*
  Important: While coming to this function, the kernel actually executes in the context
  of the process. Thus we can easily get the PID of the process which currently issued the system call. 
  */
  pid = currentThread->pid;
  k_thread = ptable[pid];
  thread_cleanup(k_thread);
  return;
}
