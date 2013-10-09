#include <defs.h>

typedef uint64_t pml4e_entry;

enum PAGE_PML4E_FLAGS{

    PML4E_PRESENT = 1,                  //Is page present in physical memory.
    PML4E_WRITABLE = 2,                 //Is page write permitted. 1 means both read and write access
    PML4E_USER = 4,                     /*Is it a user page or kernel page. 1 means both user and
                                        supervisor can read*/
    PML4E_WRITETHROUGH = 8,             /*Write back or write through cache policy for the
                                        page or page translation table to which this entry points.
                                        means writethrough cache */
    PML4E_NOT_CACHABLE = 0x10,          /*Whether the page or pagen translation table this entry
                                        points to is cachable. 0 means cahcable*/
    PML4E_ACCESSED = 0x20,              /*Whether the page or pagen translation table this entry
                                        points to is accessed */
    PML4E_4MB = 0x80,
    PML4E_AVL = 0x700,                  //Bits 9,10 and 11
    PML4E_FRAME = 0x00000FFFFFFFF000    // Target physical address which is directly stored.

};

//! sets a flag in the page table entry
extern void pml4e_entry_add_attrib (pml4e_entry* , uint64_t);

//! clears a flag in the page table entry
extern void pml4e_entry_del_attrib (pml4e_entry* , uint64_t);

//! sets a frame to page table entry
extern void pml4e_entry_set_frame (pml4e_entry*, uint64_t);

//! test if page is present
extern bool pml4e_entry_is_present (pml4e_entry);

//! test if directory is user mode
extern bool pml4e_entry_is_user (pml4e_entry);

//! test if directory contains 4mb pages
extern bool pml4e_entry_is_4mb (pml4e_entry);

//! test if page is writable
extern bool pml4e_entry_is_writable (pml4e_entry);
