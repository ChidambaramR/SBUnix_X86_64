#ifndef _STDLIB_H
#define _STDLIB_H

#include <defs.h>

int main(int argc, char* argv[], char* envp[]);
void exit(int status);
int strlen(const char*);
char* convert(uint64_t, int);
void* memset(void *, unsigned char, int);
void cls();
void update_cursor();
char* memcpy(char*, const char*, int);

#endif
