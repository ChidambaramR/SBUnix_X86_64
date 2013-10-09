#include <sys/mm/vmmgr_pdpe.h>

inline void pdpe_entry_add_attrib(pdpe_entry *e, uint64_t attrib){
    *e |= attrib;
}

inline void pdpe_entry_del_attrib(pdpe_entry *e, uint64_t attrib){
    *e &= (~attrib);
}

/*
Sets the 32 bit base address of the page table. Since it is a physical
page again it, is aligned in terms of 4KB which means the lowest three
bits are always 0. So we need not left shift the page. 
*/
inline void pdpe_entry_set_frame(pdpe_entry *e, uint64_t addr){
    *e = (*e & ~PDPE_FRAME) | addr;
}

inline bool pdpe_entry_is_present(pdpe_entry e){
    return ( e & PDPE_PRESENT );
}

inline bool pdpe_entry_is_writable(pdpe_entry e){
    return ( e & PDPE_WRITABLE );
}

inline uint32_t pdpe_entry_pfn(pdpe_entry e){
    return ( e & PDPE_FRAME );
}

inline bool pdpe_entry_is_user (pdpe_entry e) {
  return ( e & PDPE_USER );
}

inline bool pdpe_entry_is_4mb (pdpe_entry e) {
  return ( e & PDPE_4MB );
}


