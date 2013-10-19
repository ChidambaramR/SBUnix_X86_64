#include <defs.h>
#include <stdlib.h>
#include <stdio.h>

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
