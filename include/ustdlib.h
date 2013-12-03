#ifndef _USTDLIB_H_
#define _USTDLIB_H_
#include<defs.h>

char* convert_u(uint64_t, uint16_t);
uint64_t fork();
int getpid();
void sleep(uint64_t);
int read(char *);
int wait();
int execve(const char*);
uint16_t ps();
void cls();
char* memcpy(char*, const char*, uint32_t);
void strncpy(char*, const char*, uint32_t);
int strcmp(char*, char*);
int strcpsn(const char*, const char*);
int strlen(const char*);
void memset(void*, uint8_t, uint32_t);
void exit(int);
#endif
