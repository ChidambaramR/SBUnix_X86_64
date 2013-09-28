#ifndef _IO_H
#define _IO_H

static __inline void
outb (unsigned short int port, unsigned char value)
{
  __asm__ __volatile__("outb %0, %1": :"a" (value), "Nd" (port));
}

static __inline
unsigned char inb( unsigned short port )
{
  unsigned char ret;
  __asm__ __volatile__( "inb %1, %0": "=a"(ret) : "Nd"(port) );
  return ret;
}

#endif
