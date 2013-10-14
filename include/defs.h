#ifndef _DEFS_H
#define _DEFS_H

#define NULL 0
#define TEXT_COLOR 0x1F
#define MY_KERNEL_SIZE 40 // Size used by kernel in MB

typedef unsigned long __uint64_t;
typedef __uint64_t uint64_t;
typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;
typedef int __int32_t;
typedef __int32_t int32_t;
typedef unsigned short __uint16_t;
typedef __uint16_t uint16_t;
typedef unsigned char __uchar8_t;
typedef __uchar8_t uint8_t;

typedef enum
{
  FALSE,
  TRUE
} bool;
#endif
