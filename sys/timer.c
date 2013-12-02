#include <defs.h>
#include <sys/idt.h>
#include <io.h>
#include <sys/timer.h>
#include <stdio.h>
#include <sys/kthread.h>

extern void irq_install_handler(int,void *);
extern Thr_Queue runQueue;
uint16_t total_ticks = 0; // Variable to keep track of total ticks thus far
uint16_t total_ticks_secs=0;
uint16_t mins=0;
uint16_t hours=0;
uint16_t hours_units=0;
uint16_t hours_tens=0;
uint16_t minutes_units=0;
uint16_t minutes_tens=0;
uint16_t seconds_units=0;
uint16_t seconds_tens=0;
extern kthread* currentThread;

void timer_handler(regs *r){
    Thr_Queue* run_queue = &runQueue;
    kthread* crawl = run_queue->head;
    
//    uint16_t prio,max=0;
  total_ticks++;
//  kprintf("timer_handler totl_ticks=%d\n",total_ticks);
  if((total_ticks % 100) == 0){
      total_ticks_secs++;
    while(crawl && crawl != currentThread){
        if((crawl->sleeping > 0))
            crawl->sleeping--;
        crawl = crawl->next_in_ThreadQ;
    }
//      kprintf("tickes = %d\n",total_ticks_secs);
      if(total_ticks_secs >= 60){
          mins++;
          total_ticks_secs = 0;
          if(mins >= 60){
              mins = 0;
              hours++;
          }
      }
      seconds_units = (total_ticks_secs % 10);
      seconds_tens = total_ticks_secs / 10;
      minutes_units = mins % 10;
      minutes_tens = mins / 10;
      hours_units = hours % 10;
      hours_tens = hours / 10;
//      kprintf("su = %d:%d, total_tickes = %d\n",seconds_tens,seconds_units,total_ticks_secs);
      write_time(seconds_units,seconds_tens,minutes_units,minutes_tens,hours_units,hours_tens);
//    write_string(0x1F,"One second has passed\n");
  }
}

/*
 * The timer will divide its input clock of 1.19MHz (1193180Hz) by the number
 * we give in the data register to determine how much times per second to
 * fire the interrupt for tha channel. Each counter registers will hold the
 * COUNT value used by the PIT to count down 
 */
void timer_setup(int hz)
{
      int COUNT = 1193180 / hz;       /* Calculate our divisor */
      int lsb, msb;
      /*
       * 0011 0110
       * Bit 0 - BCD
       * Bits 1 to 3 - Mode 3 ( square wave generator )
       * Bits 4 to 5 - 3 - Read LSB then MSB
       * Bits 6 to 7 - Counter ( channel 0 or 1 or 2 ) 
       */
      outb(0x43, 0x36);             /* Set our command byte 0x36 */
      /*
       * The counter registers in PIT are 16 bit. However the data lines
       * D0 - D7 are only 8 bits. Thus we need to send a control word 1st
       * to the PIT. Inform it is LSB. Then send the LSB portion of the data. Then
       * inform it is MSB then send the MSB portion of the data. 
       */
      lsb = COUNT & 0xFF;
      msb = COUNT >> 8;
      outb(0x40, lsb);   /* Set low byte of divisor */
      outb(0x40, msb);     /* Set high byte of divisor */
}

void timer_install(){
  timer_setup(100);
  irq_install_handler(0, timer_handler);
}
