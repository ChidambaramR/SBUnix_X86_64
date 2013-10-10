#ifndef _STDIO_H
#define _STDIO_H

#define START_MEMORY 0xFFFFFFFF80400000 // Staring address of VGA BUFFER ( VGA MEMORY )
#define TIMER_MEMORY 0xFFFFFFFF80400F90 // Starting address of system clock
#define KEYBOARD_WARMUP_MEMORY 0xB8F8A // Starting position as mentioned in the handout
#define MAX_ROWS 25
#define MAX_COLUMNS 80

#include <defs.h>
//#include <unistd.h>

int printf(const char *format, ...);
int scanf(const char *format, ...);
void write_char(int, char);
void write_string(int, const char*);
void write_time(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void write_key(int, char);

#endif
