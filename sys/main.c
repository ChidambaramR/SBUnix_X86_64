#include <defs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/irq.h>
#include <sys/kb.h>
#include <sys/timer.h>
#include <sys/mm/regions.h>
#include <sys/mm/mmgr.h>
/*
+defs.h - Included typedefs for datatypes
*/
#include <defs.h>

extern bool get_paging_status();

void start(uint32_t* modulep, void* physbase, void* physfree)
{
        uint64_t *test,*test2,*test3,*test4,*test5,*test6;
        cls();
        printf("Screen has been cleared. In function \"%s\", its address = 0x%p \n\n",__FUNCTION__,(uint64_t)start);
        mm_phy_init(modulep);
        mmgr_print_memory_status();
        test = (uint64_t *)mmgr_alloc_block();
        test2 = (uint64_t*)mmgr_alloc_size_blocks(20);
        test3 = (uint64_t*)mmgr_alloc_block();
        mmgr_free_block(test);
        test4 = (uint64_t*)mmgr_alloc_size_blocks(2);
        test5 = (uint64_t*)mmgr_alloc_block();
        test6 = (uint64_t*)mmgr_alloc_block(1);
        printf("address of test = %p, test2 = %p test3 = %p test4 = %p test5 = %p test6 = %p\n",test,test2,test3,test4,test5,test6);
        mmgr_print_memory_status();
        printf("\n\n Is paging %d\n",get_paging_status());
	
	// kernel starts here
	while(1);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	register char *temp1, *temp2;
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);
	reload_gdt();
	setup_tss();
        reload_idt();
        irq_install();
        timer_install();
        keyboard_install();
       __asm__("sti");

	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	for(
		temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)0xb8000;
		*temp1;
		temp1 += 1, temp2 += 2
	) *temp2 = *temp1;
	while(1);
}
