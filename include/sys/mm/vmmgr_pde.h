#include <defs.h>

typedef uint64_t pd_entry;

enum PAGE_PDE_FLAGS{
 
    PDE_PRESENT = 1,                  //Is page present in physical memory.
    PDE_WRITABLE = 2,                 //Is page write permitted. 1 means both read and write access
    PDE_USER = 4,                     /*Is it a user page or kernel page. 1 means both user and
                                        supervisor can read*/
    PDE_WRITETHROUGH = 8,             /*Write back or write through cache policy for the
                                        page or page translation table to which this entry points.
                                        means writethrough cache */
    PDE_NOT_CACHABLE = 0x10,          /*Whether the page or pagen translation table this entry
                                        points to is cachable. 0 means cahcable*/
    PDE_ACCESSED = 0x20,              /*Whether the page or pagen translation table this entry
                                        points to is accessed */
    PDE_4MB = 0x80,
    PDE_AVL = 0x700,                  //Bits 9,10 and 11
    PDE_FRAME = 0x00000FFFFFFFF000    // Target physical address which is directly stored.
 
};

//! sets a flag in the page table entry
extern void pd_entry_add_attrib (pd_entry* e, uint64_t attrib);

//! clears a flag in the page table entry
extern void pd_entry_del_attrib (pd_entry* e, uint64_t attrib);

//! sets a frame to page table entry
extern void pd_entry_set_frame (pd_entry*, uint64_t);

//! test if page is present
extern bool pd_entry_is_present (pd_entry);

//! test if directory is user mode
extern bool pd_entry_is_user (pd_entry);

//! test if directory contains 4mb pages
extern bool pd_entry_is_4mb (pd_entry);

//! test if page is writable
extern bool pd_entry_is_writable (pd_entry e);

//! get page table entry frame address
extern  uint32_t pd_entry_pfn (pd_entry e);
