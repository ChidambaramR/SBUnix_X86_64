#ifndef _ISR_H_
#define _ISR_H

#include <defs.h>

void page_fault_handler(uint64_t);
void general_protection_fault_handler(uint64_t);

#endif
