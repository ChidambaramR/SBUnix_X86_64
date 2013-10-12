/*
The kernel represents a process's address space with a data structure called the 
memory descriptor. This structure contains all the information related to the process 
address space.
*/
#ifndef _MEMDESC_H
#define _MEMDESC_H

struct vos {
  void (*open)(struct vm_area_struct *);
  void (*close)(struct vm_area_struct *);
  void (*unmap)();
  void (*protect)();
  void (*sync)();
  unsigned long (*nopage)(struct vm_area_struct *, unsigned long address, unsigned long page, int write_access);
  void (*swapout)(struct vm_area_struct *, unsigned long, pte_t *);
  pte_t (*swapin)(struct vm_area_struct *, unsigned long, unsigned long);
};

typedef struct vos vm_operations_struct;

typedef uint64_t pgprotval_t;

typedef struct pgprot {
  pgprotval_t pgprot;
} pgprot_t;

struct vmas {
  mm_struct *vm_mm;
  uint64_t vm_start, vm_end; // Start and end of the region
  uint16_t vm_flags;
  pgprot_t vm_page_prot; // protection attributes for this region  
  vm_operations_struct *vm_ops;
};

typedef struct vmas vm_area_struct;

/* 
-- The mm_users field is the number of processes using this address space. 
For example, if two threads share this address space, mm_users is equal to two.

-- The mm_count field is the primary reference count for the mm_struct.All mm_users 
equate to one increment of mm_count.Thus, in the previous example, mm_count is only one.
If nine threads shared an address space, mm_users would be nine, but again 
mm_count would be only one. Only when mm_users reaches zero (when all threads using an 
address space exit) is mm_count decremented.When mm_count finally reaches zero, 
there are no remaining references to

-- When the kernel operates on an address space and needs to
bump its associated reference count, the kernel increments mm_count

-- Having two counters enables the kernel to differentiate between the main usage 
counter (mm_count) and the number of processes using the address space (mm_users)

-- The mmap stores in linked list all the memory areas in this address space

-- All of the mm_struct structures are strung together in a doubly linked list via the
mmlist field.The initial element in the list is the init_mm memory descriptor, which
describes the address space of the init process
*/

struct mms {
  int mm_count; // Number of process sharing this descriptor
  int mm_users;
  pml4* pgd;
  uint64_t context;
  uint64_t start_code, end_code, start_data, end_data;
  uint64_t start_brk, brk, start_stack, start_mmap;
  uint64_t arg_start, arg_end, env_start, env_end;
  uint64_t rss; // No of pages resident in memory
  uint64_t total_vm; // Total number of bytes in the address space
  uint64_t locked_vm; // Total number of bytes locked in memory
  uint64_t def_flags; // status to use when memory regions are created
  struct list_head mmlist; // list of all mm_structs
  vm_area_struct *mmap; // Pointer to first region desc
  vm_area_struct *mmap_avl; // Faster search of region desc
  vm_area_struct *mmap; // list of VMA's
};

typedef struct mms mm_struct;
#endif

