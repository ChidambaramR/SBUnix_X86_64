#ifndef _ISR_H_
#define _ISR_H

#include <defs.h>
#include <sys/kthread.h>
#include <sys/idt.h>

void page_fault_handler(uint64_t, void*);
void general_protection_fault_handler(uint64_t);
void write(const char*);
int sys_getpid();
void sys_exit();
int fork(regs*);
void sleep(uint64_t time);
int doread(char*, int);
void wait();
int do_execve();
void print_process();
void do_cls();
uint16_t do_ls();
int do_sbrk(int);
int do_open(char*);
int do_close(int);
int changed(char*);
void do_pwd();
#endif
