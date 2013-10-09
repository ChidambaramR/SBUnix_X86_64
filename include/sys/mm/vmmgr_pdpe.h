#include <defs.h>

typedef uint64_t pdpe_entry;

enum PAGE_PDPE_FLAGS{

    PDPE_PRESENT = 1,                  //Is page present in physical memory.
    PDPE_WRITABLE = 2,                 //Is page write permitted. 1 means both read and write access
    PDPE_USER = 4,                     /*Is it a user page or kernel page. 1 means both user and
                                        supervisor can read*/
    PDPE_WRITETHROUGH = 8,             /*Write back or write through cache policy for the
                                        page or page translation table to which this entry points.
                                        means writethrough cache */
    PDPE_NOT_CACHABLE = 0x10,          /*Whether the page or pagen translation table this entry
                                        points to is cachable. 0 means cahcable*/
    PDPE_ACCESSED = 0x20,              /*Whether the page or pagen translation table this entry
                                        points to is accessed */
    PDPE_4MB = 0x80,
    PDPE_AVL = 0x700,                  //Bits 9,10 and 11
    PDPE_FRAME = 0x00000FFFFFFFF000    // Target physical address which is directly stored.

};

//! sets a flag in the page table entry
extern void pdpe_entry_add_attrib (pdpe_entry* , uint64_t);

//! clears a flag in the page table entry
extern void pdpe_entry_del_attrib (pdpe_entry* , uint64_t);

//! sets a frame to page table entry
extern void pdpe_entry_set_frame (pdpe_entry*, uint64_t);

//! test if page is present
extern bool pdpe_entry_is_present (pdpe_entry);

//! test if directory is user mode
extern bool pdpe_entry_is_user (pdpe_entry);

//! test if directory contains 4mb pages
extern bool pdpe_entry_is_4mb (pdpe_entry);

//! test if page is writable
extern bool pdpe_entry_is_writable (pdpe_entry);

