#include <io.h>
#include <defs.h>
#include <sys/idt.h>

extern void _irq0();
extern void _irq1();
extern void _irq2();
extern void _irq3();
extern void _irq4();
extern void _irq5();
extern void _irq6();
extern void _irq7();
extern void _irq8();
extern void _irq9();
extern void _irq10();
extern void _irq11();
extern void _irq12();
extern void _irq13();
extern void _irq14();
extern void _irq15();

void idt_set_gate(unsigned char, uint64_t, uint16_t, unsigned char);

/* This array is actually an array of function pointers. We use
 * *  this to handle custom IRQ handlers for a given IRQ */
void *irq_routines[16] =
{
      0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0
};

/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(regs *r)){
      irq_routines[irq] = handler;
}

/* This clears the handler for a given IRQ */
void irq_uninstall_handler(int irq){
      irq_routines[irq] = 0;
}

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
 * *  is a problem in protected mode, because IDT entry 8 is a
 * *  Double Fault! Without remapping, every time IRQ0 fires,
 * *  you get a Double Fault Exception, which is NOT actually
 * *  what's happening. We send commands to the Programmable
 * *  Interrupt Controller (PICs - also called the 8259's) in
 * *  order to make IRQ0 to 15 be remapped to IDT entries 32 to
 * *  47 */

/*
 * IMPORTANT - PORT BIT MAPPINGS OF THE PIC
 * 0x20 - Primary PIC command and status register
 * 0x21 - Primary PIC Interrupt mask register and data register
 * 0xA0 - Secondary PIC command and data register
 * oxA1 - Secondary PIC Inteerupt mask register and data register
 */

void irq_remap(){
      /*
       * ICW1
       * 0x11 - 00010001 LSB is bit 0
       * Bit 0 - Send ICW4
       * Bit 1 - This is 1, if there is only 1 PIC in the system. 0 if cascaded
       * Bit 2
       * Bit 3 - 1 if level triggered mode. 0 if egde triggered mode
       * Bit 4 - 1 if initialized
       * Bit 5,6,7 - Should be 0
       */
      outb(0x20, 0x11);
      outb(0xA0, 0x11);

      /*
       * ICW2
       * Map the base of interrupt number in the Primary PIC and the secondary PIC. 
       * 0x21 - Primary PIC. 0 is mapped to 32(decimal) - 0x20
       * 0xA1 - Secondary PIC. 8 is mapped to 40(decimal) - 0x28
       */
      outb(0x21, 0x20);
      outb(0xA1, 0x28);
      
      /*
       * We send ICW3 to both PIC's to let each other know which IRQ line is used
       * for both master and associated PC's. 80x86's uses IRQ 2 to connect the
       * master PIC to the slave PIC.
       * For master PIC, bits 0 - 7 specifies what IRQ line is connected to the
       * slave PIC. i.e bit 2 is to be passed. i.e 0000 0100. this is 0x04
       * For secondary PIC, it is binary representation. So IRQ 2 is 0000 0010
       * which is 0x02
       */
      outb(0x21, 0x04);
      outb(0xA1, 0x02);
      
      /*
       * Bit 0 - if set, 80x86 mode
       * Bit 1 - if set, on the last interrupt acknowledge pulse, the controller
       * automatically performs EOI operation.
       * Bit 2 tp 7 - Not necessary for us
       */
      outb(0x21, 0x01);
      outb(0xA1, 0x01);
      
      /*
       * All is done. NULL out the data registers
       */
      outb(0x21, 0x0);
      outb(0xA1, 0x0);


}

/* We first remap the interrupt controllers, and then we install
 * *  the appropriate ISRs to the correct entries in the IDT. This
 * *  is just like installing the exception handlers */
void irq_install()
{
      irq_remap();

      idt_set_gate(32, (uint64_t)_irq0, 0x08, 0x8E);
      idt_set_gate(33, (uint64_t)_irq1, 0x08, 0x8E);
      idt_set_gate(34, (uint64_t)_irq2, 0x08, 0x8E);
      idt_set_gate(35, (uint64_t)_irq3, 0x08, 0x8E);
      idt_set_gate(36, (uint64_t)_irq4, 0x08, 0x8E);
      idt_set_gate(37, (uint64_t)_irq5, 0x08, 0x8E);
      idt_set_gate(38, (uint64_t)_irq6, 0x08, 0x8E);
      idt_set_gate(39, (uint64_t)_irq7, 0x08, 0x8E);
      idt_set_gate(40, (uint64_t)_irq8, 0x08, 0x8E);
      idt_set_gate(41, (uint64_t)_irq9, 0x08, 0x8E);
      idt_set_gate(42, (uint64_t)_irq10, 0x08, 0x8E);
      idt_set_gate(43, (uint64_t)_irq11, 0x08, 0x8E);
      idt_set_gate(44, (uint64_t)_irq12, 0x08, 0x8E);
      idt_set_gate(45, (uint64_t)_irq13, 0x08, 0x8E);
      idt_set_gate(46, (uint64_t)_irq14, 0x08, 0x8E);
      idt_set_gate(47, (uint64_t)_irq15, 0x08, 0x8E);
}

void irq_handler(regs *r)
{
      /* This is a blank function pointer */
      void (*handler)(regs *r);

      /* Find out if we have a custom handler to run for this
       IRQ, and then finally, run it.
       Since we have mapped the interrupts from 32 to 47, we are
       subtracting it by 32. So IRQ 0 is mapped to IDT 32. 
       irq_routines is a table which contains the function pointers
       for the functions which handle the actual hardware interrupts. 
       */
      handler = irq_routines[r->intNo - 0x32];
      if (handler){
            handler(r);
      }

      /* If the IDT entry that was invoked was greater than 40
      (meaning IRQ8 - 15), then we need to send an EOI to
      the slave controller */
      if (r->intNo >= 40){
            outb(0xA0, 0x20);
      }

      /* In either case, we need to send an EOI to the master
      interrupt controller too */
      outb(0x20, 0x20);
}
