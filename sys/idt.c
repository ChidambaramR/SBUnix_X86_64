#include <sys/idt.h>
#include <sys/isr.h>
#include <sys/kthread.h>

#define MAX_IDT 256

extern void _isr0();
extern void _isr1();
extern void _isr13();
extern void _isr14();
extern void _isr80();
extern void *memset(void*,int,int);
extern void write_string(int, const char *);
extern void _x86_64_asm_ltr(void*);
extern kthread* *ptable;
idtE idt[MAX_IDT];

static struct idtr_t idtr = {
  sizeof(idt) - 1,
  (uint64_t)idt,
};

void _x86_64_asm_lidt(struct idtr_t* idtr);

int fault_handler(regs *r)
{
      /* Is this a fault whose number is from 0 to 31? */
      void* faulting_instruction;
      uint16_t callNo;
      const char* str;
      int val=1;
//      kthread* k_thread;
      //asm volatile("leaq (%%rip), %0;": "=r"(faulting_instruction));
      asm volatile("movq %%rsp, %0" : "=r" (faulting_instruction));
      if (r->intNo <= 0x80){
          switch(r->intNo){
              case 0xE: page_fault_handler(r->errCode,faulting_instruction);
                        break;
              case 0xD: general_protection_fault_handler(r->errCode);
                        break;
              case 0x80:callNo = r->rax;
                        switch(callNo){
                          case 1: 
                                  sys_exit();
                                  break;
                          case 2: str = (const char*)r->rdi;
                                  write(str);
                                  break;
                          case 20: val = sys_getpid();
                                  break;
                        } 
                        break;
              default:  write_string(0x1F," Unknown Exception. System Halted!\n");
                        while(1);
          }
//         write_string(0x1F," Exception. System Halted!\n");
//         for (;;);
      }
      return val;
}

void idt_set_gate(unsigned char number, uint64_t base, uint16_t selector, unsigned char flags) {
  
  /* Set Base Address */
  /* Set Base Address */
  idt[number].baseLow = base & 0xFFFF;
  idt[number].baseMid = (base >> 16) & 0xFFFF;
  idt[number].baseHigh = (base >> 32) & 0xFFFFFFFF;

  /* Set Selector */
  idt[number].selector = selector;
  idt[number].flags = flags;
                    
  /* Set Reserved Areas to Zero */
  idt[number].reservedIst = 0;
  idt[number].reserved = 0;
}

void x86_64_asm_ltr(){
  uint16_t val = 0x2B;
  _x86_64_asm_ltr(&val);
}

void reload_idt() {
  /* Clear out the entire IDT, initializing it to zeros */
  memset(&idt, 0, sizeof(struct idtEntry) * 256);
  idt_set_gate(0, (uint64_t)_isr0, 0x08, 0x8E);
  idt_set_gate(1, (uint64_t)_isr1, 0x08, 0x8E);
  idt_set_gate(0xD, (uint64_t)_isr13, 0x08, 0x8E);
  idt_set_gate(0xE, (uint64_t)_isr14, 0x08, 0x8E);
  idt_set_gate(0x80, (uint64_t)_isr80, 0x08, 0xEE);
  _x86_64_asm_lidt(&idtr);
  x86_64_asm_ltr();
}

