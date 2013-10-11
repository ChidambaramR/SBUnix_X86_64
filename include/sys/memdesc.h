#ifndef _MEMDESC_H
#define _MEMDESC_H

struct mms {
  vm_area_struct *mmap; // list of VMA's
};
#endif

/*
The kernel represents a process's address space with a data structure called the 
memory descriptor. This structure contains all the information related to the process 
address space.
*/
