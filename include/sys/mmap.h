#ifndef _MMAP_H_
#define _MMAP_H_
#include <defs.h>
#include <sys/kthread.h>

void mmap(void*, uint32_t, int, int, int, uint64_t, kthread*);

#endif
