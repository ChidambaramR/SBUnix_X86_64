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
#define UserHeap  0x000000000080000
#define VmaStart  0x0000000000400000

extern uint64_t USER_CS;
extern uint64_t USER_DS;
extern global_thread_list allThreadList;
extern Thr_Queue runQueue;
extern void switch_to_user();
extern uint64_t get_cr3_register();
extern kthread* currentThread;
extern uint64_t kernel_pgd;
extern int debug;
extern void PushU(kthread*, uint64_t);
extern void reload_cr3(uint64_t);
/*init_user_memory(){
    pte* new_page_table = get_new_page_table((pml4*)(currentTask->cr3 & 0xFFFFF000), user_code);
    
}*/


void PushU_General_Registers(kthread* k_thread){

    PushU(k_thread, 0);  /* rax */
    PushU(k_thread, 0);  /* rbx */
    PushU(k_thread, 0);  /* rcx */
    PushU(k_thread, 0);  /* rdx */
    PushU(k_thread, 0);  /* rsi */
    PushU(k_thread, 0);  /* rdi */
    PushU(k_thread, 0);  /* rbp */
    PushU(k_thread, 0);  /* r15 */
    PushU(k_thread, 0);  /* r14 */
    PushU(k_thread, 0);  /* r13 */
    PushU(k_thread, 0);  /* r12 */
    PushU(k_thread, 0);  /* r11 */
    PushU(k_thread, 0);  /* r10 */
    PushU(k_thread, 0);  /* r09 */
    PushU(k_thread, 0);  /* r08 */
}


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
    PushU(k_thread, (uint64_t)USER_DS);

    PushU(k_thread, (uint64_t)(k_thread->rsp));

    // PushU rflags
    PushU(k_thread, (uint64_t)0x200);

    // PushU kernel CS. Needed for iretq call
    PushU(k_thread, (uint64_t)USER_CS);

    //PushU the address of launching function
    PushU(k_thread, (uint64_t) startFunc);

    /*
    PushU fake error code and interrupt number. Why? Because, in the irq handling function
    we have pushed two 8 byte values. One is the error code and other is the interrupt number.
    Actually this is not needed. Since IRQ's are already built, this is sort of a HACk to make
    it working. A corresponding add $0x10, %rsp statement will be found while popping the registers.
    */
    PushU(k_thread, (uint64_t)0);
    PushU(k_thread, (uint64_t)0);

    /*
    PushU initial values for general-purpose registers. This is just like anyother register
    saving mechanism.
    */
    PushU_General_Registers(k_thread);

}


static void Init_Thread_user(kthread* k_thread,const char* name, void* stackPage, uint16_t prio, bool detached){
    kthread* owner = detached ? (kthread*)0: currentThread;
    k_thread->stackPage = stackPage;
    /*
    Every user thread has a kernel stack. The reason being, when an interrupt occurs, 
    if there is no kernel stack for each user thread, then on an interrupt we will write
    into other kernel thread's stack. 
    */
    //k_thread->kstack = (void*)sub_malloc(0,1);
    k_thread->rsp = ((uint64_t) k_thread->stackPage) + VIRT_PAGE_SIZE - 0x8;
    k_thread->krsp = ((uint64_t) k_thread->kstack) + 2*VIRT_PAGE_SIZE - 0x8;
    k_thread->kstack = (void*)(((uint64_t)k_thread->kstack) + 2*VIRT_PAGE_SIZE - 0x8);
    k_thread->priority = prio;
    //k_thread->userContext = 0;
    k_thread->owner = owner;
    k_thread->refCount = detached ? 1 : 2;
    k_thread->kernel_thread = 0;
    k_thread->no_stack_pages = 1;
    k_thread->alive = TRUE;
    k_thread->sleeping = 0;
    k_thread->name = name;
    k_thread->pcr3 = get_cr3_register();
    k_thread->cr3 = (uint64_t)currentThread->cr3;
    if(k_thread != currentThread){
       k_thread->pid = alloc_pid();
    }
}

kthread* create_kthread_user(const char* name, int prio, bool detached){
    kthread* k_thread;
    void* stackPage = 0;
    k_thread = (kthread*)sub_malloc(sizeof(kthread),0);
    if( !k_thread )
        return NULL;
    stackPage = (void*)UserStack; 
    k_thread->kstack = (void*)sub_malloc(1,1);
    if( !k_thread->kstack ){
        sub_free(k_thread->kstack);
        return NULL;
    }
    debug = 1;
    /*
     * Initialize the stack pointer of the new thread
     * and accounting info
     */
    Init_Thread_user(k_thread, name, stackPage, prio, detached);

    /* Add to the list of all threads in the system. */
    append_global_list_queue(&allThreadList, k_thread);

    return k_thread;
}




void create_new_task(kthread* k_thread, void* startFunc, const char* name, uint16_t arg, uint16_t priority, bool detached){
    setup_kthread_user(k_thread, startFunc, arg);
    return;
}

uint32_t do_exec(char *name){
    struct exec executable[20];
    uint64_t currentStack_page, currentHeap_page;
    uint16_t pgm_entries;
    kthread* k_thread;
  
    uint16_t i;
    uint64_t entry_point;
   //` printf("in exec\n");
    if( readelf(name, executable, &pgm_entries, &entry_point) ){
        k_thread = create_kthread_user(name, 10, 1);
        if(!k_thread)
            return NULL;
        //cls();
        for(i=0; i < pgm_entries; i++){
            mmap((void*)executable[i].seg_page_start, executable[i].seg_length, 0, 0, 0, 0, k_thread);
            memcpy((char*)executable[i].seg_actual_start, executable[i].seg_mem, executable[i].seg_length);    
    
        }

        currentStack_page = UserStack;
        mmap((void*)currentStack_page, VIRT_PAGE_SIZE, 0, 0, 0, 0, k_thread);
        currentHeap_page = UserHeap;
        mmap((void*)currentHeap_page, VIRT_PAGE_SIZE, 0, 0, 0, 0, k_thread);
        k_thread->brk = (int)currentHeap_page;
        // Create the actual task structure
        create_new_task(k_thread, (void*)entry_point, "first", 0, 10, 1);
        add_to_ptable(k_thread);
        disable_interrupts();
        runnable_kthread(k_thread);
        enable_interrupts();
        return 1;
    }
    else
        return 0;
}

void do_exec1(char* name){
    struct exec executable[20];
    uint64_t currentStack_page, currentHeap_page;
    uint16_t pgm_entries;
    uint16_t i;
    uint64_t entry_point;
    //printf("in exec1");
    Init_Thread_user(currentThread, name, (void*)UserStack, 10, 1);
    append_global_list_queue(&allThreadList, currentThread);
    if( readelf(name, executable, &pgm_entries, &entry_point) ){
        for(i=0; i < pgm_entries; i++){
            mmap((void*)executable[i].seg_page_start, executable[i].seg_length, 0, 0, 0, 0, currentThread);
            memcpy((char*)executable[i].seg_actual_start, executable[i].seg_mem, executable[i].seg_length);

        }

        currentStack_page = UserStack;
        mmap((void*)currentStack_page, VIRT_PAGE_SIZE, 0, 0, 0, 0, currentThread);
        currentHeap_page = UserHeap;
        mmap((void*)currentHeap_page, VIRT_PAGE_SIZE, 0, 0, 0, 0, currentThread);
        currentThread->brk = (int)currentHeap_page;

        // Create the actual task structure
        create_new_task(currentThread, (void*)entry_point, name, 0, 10, 1);
        disable_interrupts();
        runnable_kthread(currentThread);
        enable_interrupts();
    }

}
void clear_vmas(kthread* k_thread){
    vm_area_struct *crawl = k_thread->mmap, *tmp;
    while(crawl){
      tmp = crawl->vm_next;
      memset((char*)crawl, 0, sizeof(vm_area_struct));
      sub_free(crawl);
      crawl = tmp;
    }
    k_thread->mmap = k_thread->mmap_cache = NULL;
}

void main_execve(char* name){
    char temp[30];
    memset(temp, 0, sizeof(temp));
    strncpy(temp, name, strlen(name));
    clear_vmas(currentThread);
    clear_page_tables(currentThread->cr3);
    reload_cr3(currentThread->pcr3);
    do_exec1(temp);
    //printf("finisheed\n");
}
