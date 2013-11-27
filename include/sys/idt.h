#ifndef _IDT_H
#define _IDT_H

#include <defs.h>

/*
 * Interrupt Descriptor Structure
 * Refer to Intel Manual section 6.14.2. The 64-bit RIP referenced by interrupt-gate
 * descriptors allows an interrupt service routine to be located anywhere in the
 * linear-address space
 * */
typedef struct idtEntry {
      uint16_t   baseLow;
      uint16_t   selector;
      unsigned char   reservedIst;
      unsigned char   flags;
      uint16_t   baseMid;
      uint32_t   baseHigh;
      uint32_t   reserved;
} __attribute__((packed)) idtE;


typedef struct registers{
    //uint64_t ds;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t intNo, errCode;
    uint64_t rip, cs, rflags, rsp, ss;
    } regs;

struct idtr_t {
    uint16_t size;
    uint64_t addr;
    }__attribute__((packed));

void reload_idt();


/*
 * The actual ISR's. Its address should be stored in IDT's base field.
 * So we will pass the function address 
 */
//void isr0();
//void isr1();

#endif
