#include <defs.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mm/vmmgr_virtual.h>
#include <sys/mm/mmgr.h>
#include <sys/task.h>
#include <elf.h>
#include <sys/mmap.h>
#include <sys/kthread.h>

#define UserCode  0x0000000000400000
#define UserData  0x0000000000600000
#define UserStack 0x0000000000bff000
#define VmaStart  0x0000000000400000

extern uint64_t USER_CS;
extern uint64_t USER_DS;
extern global_thread_list allThreadList;
extern Thr_Queue runQueue;
extern void switch_to_user();
extern uint64_t get_cr3_register();
extern kthread* currentThread;



/*init_user_memory(){
    pte* new_page_table = get_new_page_table((pml4*)(currentTask->cr3 & 0xFFFFF000), user_code);
    
}*/

void setup_kthread_user(kthread* k_thread, void* startFunc, uint16_t arg){

    /*
    When you cause a software interrupt, the flags register is pushed followed by CS and IP.
    The iret instruction does the reverse, it pops IP, CS and then the flags register
    IRETQ pops in this order (rIP, CS, rFLAGS, rSP, and SS). So we are preparing the
    stack of the new thread in such a way that after IRETQ pops, it will directly go to the
    startFunc ( above ). That is, an ideal stack before iretq will be as follows.

    (gdb) x /16wx $rsp
    0xffffffff81404fc0:     0x80200a14      0xffffffff      0x00000008      0x00000000
    0xffffffff81404fd0:     0x00000000      0x00000000      0x81404fe0      0xffffffff
    0xffffffff81404fe0:     0x00000010      0x00000000      0x802010a4      0xffffffff

    0xffffffff80200a14 is the RIP, 8 is the CS, 0 is the RFLAGS, 0xffffffff81404fe0 is the
    RSP and 0x10 is the DS. If you see, it is in the order as popped by IRETQ.
    */
    Push(k_thread, (uint64_t)USER_DS);

    Push(k_thread, (uint64_t)(k_thread->rsp));

    // Push rflags
    Push(k_thread, (uint64_t)0x200);

    // Push kernel CS. Needed for iretq call
    Push(k_thread, (uint64_t)USER_CS);

    //Push the address of launching function
    Push(k_thread, (uint64_t) startFunc);

    /*
    Push fake error code and interrupt number. Why? Because, in the irq handling function
    we have pushed two 8 byte values. One is the error code and other is the interrupt number.
    Actually this is not needed. Since IRQ's are already built, this is sort of a HACk to make
    it working. A corresponding add $0x10, %rsp statement will be found while popping the registers.
    */
    Push(k_thread, (uint64_t)0);
    Push(k_thread, (uint64_t)0);

    /*
    Push initial values for general-purpose registers. This is just like anyother register
    saving mechanism.
    */
    Push_General_Registers(k_thread);

    /*
    We need this? Probably for ring3 switch
    Push values for saved segment registers.
    Only the ds and es registers will contain valid selectors.
    The fs and gs registers are not used by any instruction
    generated by gcc.
    */
   // Push(k_thread, 2<<3);  /* ds */
   // Push(k_thread, 2<<3);  /* es */
   // Push(k_thread, 0);  /* fs */
   // Push(k_thread, 0);  /* gs */


}


static void Init_Thread_user(kthread* k_thread,const char* name, void* stackPage, uint16_t prio, bool detached){
    kthread* owner = detached ? (kthread*)0: currentThread;
    k_thread->stackPage = stackPage;
    k_thread->rsp = ((uint64_t) k_thread->stackPage) + VIRT_PAGE_SIZE;
    k_thread->numTicks = 0;
    k_thread->priority = prio;
    //k_thread->userContext = 0;
    k_thread->owner = owner;
    k_thread->refCount = detached ? 1 : 2;
    k_thread->kernel_thread = 0;
    k_thread->alive = TRUE;
    k_thread->name = name;
    init_thread_queue(&(k_thread->joinQueue));
    k_thread->pid = alloc_pid();
}

kthread* create_kthread_user(const char* name, int prio, bool detached){
    kthread* k_thread;
    void* stackPage = 0;
    k_thread = (kthread*)sub_malloc(0,1);
    if( !k_thread )
        return NULL;
    stackPage = (void*)UserStack; 
    /*
     * Initialize the stack pointer of the new thread
     * and accounting info
     */
    Init_Thread_user(k_thread, name, stackPage, prio, detached);

    /* Add to the list of all threads in the system. */
    append_global_list_queue(&allThreadList, k_thread);

    return k_thread;
}




kthread* create_new_task(void* startFunc, const char* name, uint16_t arg, uint16_t priority, bool detached){
    kthread* k_thread = create_kthread_user(name, priority, detached);
    if(!k_thread)
        return NULL;
    setup_kthread_user(k_thread, startFunc, arg);
    return k_thread;
}

uint32_t do_exec(/*char *name, char *environment*/){
   // uint64_t *data;
    /*uint64_t currentCode_page;
    uint64_t currentCode_len;
    uint64_t currentData_page;
    uint64_t currentData_len;
    uint64_t new_pte;*/
    struct exec executable[20];
    uint64_t currentStack_page;
    uint16_t pgm_entries;
    uint64_t new_pte;
    kthread* k_thread;
//    task_struct* task;
  
   // uint32_t size;
   // int codeLen=10, dataLen;
    uint16_t i;
    uint64_t entry_point;
   // char *codeBuf, *dataBuf;
   // struct posix_header_ustar executable;
   // char *prog_name = (char*)sub_malloc(strlen(name),1);
   // strcpy(prog_name, name);
    if( readelf(executable, &pgm_entries, &entry_point) ){
        // Initialize code page
        for(i=0; i < pgm_entries; i++){
            printf(" seg length %d, page %x, bug %p\n",executable[i].seg_length, executable[i].seg_page_start, executable[i].seg_mem);
            mmap((void*)executable[i].seg_page_start, executable[i].seg_length, 0, 0, 0, 0);
            // Need to ask Prof about this
//            if(i == 0)
//                memcpy((char*)entry_point, executable[i].seg_mem, executable[i].seg_length);   
//            else 
                memcpy((char*)executable[i].seg_actual_start, executable[i].seg_mem, executable[i].seg_length);    
    
        }

        // Initialize stack page
        currentStack_page = UserStack;
        new_pte = (uint64_t)mmgr_alloc_block();
        vmmgr_map_page_after_paging(new_pte, currentStack_page, 1);
        memset((void*)currentStack_page, 0, sizeof(VIRT_PAGE_SIZE));

        // Create the actual task structure
        k_thread = create_new_task((void*)entry_point, "first", 0, 10, 1);
        add_to_ptable(k_thread);
        disable_interrupts();
        runnable_kthread(k_thread);
        enable_interrupts();
        //switch_to_user();
//        Schedule();
        /*currentPage = UserCode;
        currentPage_size = usercode_len;
        init_user_memory();
        */
    }
    return 1;
}
