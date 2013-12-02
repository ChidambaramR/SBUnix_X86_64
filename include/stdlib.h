#ifndef _STDLIB_H
#define _STDLIB_H

#include <defs.h>

//int main(int argc, char* argv[], char* envp[]);
void exit(int status);
int strlen(const char*);
char* convert(uint64_t, int);
void* memset(void *, unsigned char, uint32_t);
void cls();
void update_cursor();
char* memcpy(char*, const char*, uint32_t);
void strncpy(char*, const char*, uint32_t);
int strcmp(char*, char*);
uint32_t my_atoi(char*);
uint32_t my_atool(char*);
int main(int argc, char* argv[], char* envp[]);
void exit(int status);
int strcpsn(const char*, const char*);
#endif
