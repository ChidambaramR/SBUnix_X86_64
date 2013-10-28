#include <defs.h>
#include <sys/kthread.h>
#include <sys/mm/vmmgr_virtual.h>
#include <stdlib.h>
#include <stdio.h>

uint64_t KERN_CS=0x8;
uint64_t KERN_DS=0x10;
extern void Switch_To_Thread(kthread*);
extern uint64_t get_flag_register();
extern char* stack; 
extern void start(uint16_t);
extern bool is_scheduler_on;
/*
 * List of all threads in the system.
 */
static global_thread_list allThreadList;

/*
 * Queue of runnable threads.
 */
static Thr_Queue runQueue;

/*
 * Current thread.
 */
kthread* currentThread;

static __inline__ void disable_interrupts(void)
{
    __asm__ __volatile__ ("cli");
}

static __inline__ void enable_interrupts(void)
{
    __asm__ __volatile__ ("sti");
}

/*
Add it to the end of the run queue. The scheduler function choosed which thread
can be run only from this run queue.
*/
static __inline__ void append_run_queue(Thr_Queue *listPtr, kthread *nodePtr) {                                           
    nodePtr->next_in_ThreadQ = 0;                                 
    if (listPtr->tail == 0) {                                                                   
        listPtr->head = listPtr->tail = nodePtr;                                                
        nodePtr->prev_in_ThreadQ = 0;                                                               
    }                                                                                           
    else {                                                                                      
        listPtr->tail->next_in_ThreadQ = nodePtr;                                                   
        nodePtr->prev_in_ThreadQ = listPtr->tail;                                                   
        listPtr->tail = nodePtr;                                                          
    }                                 
} 

static __inline__ void insert_run_queue(Thr_Queue *listPtr, kthread *nodePtr) {  
    nodePtr->prev_in_ThreadQ = 0;
    if (listPtr->head == 0) {
        listPtr->head = listPtr->tail = nodePtr;
        nodePtr->next_in_ThreadQ = 0;
    } else {                                           
        listPtr->head->prev_in_ThreadQ = nodePtr;     
        nodePtr->next_in_ThreadQ = listPtr->head;            
        listPtr->head = nodePtr;          
    }                                                                                           
} 

static __inline__ void append_global_list_queue(global_thread_list *listPtr, kthread *nodePtr) {                                           
    nodePtr->next_in_ThreadList = 0;                                 
    if (listPtr->tail == 0) {                                                                   
        listPtr->head = listPtr->tail = nodePtr;                                                
        nodePtr->prev_in_ThreadList = 0;                                                               
    }                                                                                           
    else {                                                                                      
        listPtr->tail->next_in_ThreadList = nodePtr;                                                   
        nodePtr->prev_in_ThreadList = listPtr->tail;                                                   
        listPtr->tail = nodePtr;                                                          
    }                                 
} 

static __inline__ void insert_global_list_queue(global_thread_list *listPtr, kthread *nodePtr) {  
    nodePtr->prev_in_ThreadList = 0;
    if (listPtr->head == 0) {
        listPtr->head = listPtr->tail = nodePtr;
        nodePtr->next_in_ThreadList = 0;
    } else {                                           
        listPtr->head->prev_in_ThreadList = nodePtr;     
        nodePtr->next_in_ThreadList = listPtr->head;            
        listPtr->head = nodePtr;          
    }                                                                                           
}

/*
We are removing a runnable thread because, it has been scheduled and thus it should not
be in the run queue. 
*/
void remove_runnable_kthread(Thr_Queue* run_queue, kthread* runnable){
    if(runnable->prev_in_ThreadQ != 0)
        runnable->prev_in_ThreadQ->next_in_ThreadQ = runnable->next_in_ThreadQ;
    else
        run_queue->head = runnable->next_in_ThreadQ;

    if(runnable->next_in_ThreadQ != 0)
        runnable->next_in_ThreadQ->prev_in_ThreadQ = runnable->prev_in_ThreadQ;
    else
        run_queue->tail = runnable->prev_in_ThreadQ;
    
    
}

/*
Parses the runQueue linked list and gets the one with maximum priority. If two or mote
thread's prioritues are same, then it choosing in the FCFS basis.
*/
kthread* next_runnable_kthread(){
    Thr_Queue* run_queue = &runQueue;
    kthread* crawl = run_queue->head;
    kthread* runnable=0;
    uint16_t prio,max=0;
    while(crawl){
        prio = crawl->priority;
        if(prio > max){
            max = prio;
            runnable = crawl;
        }
        crawl = crawl->next_in_ThreadQ;
    }
    if(!runnable)
      PANIC(__FUNCTION__,__LINE__,"NULL!!");
    remove_runnable_kthread(run_queue,runnable);
    return runnable;
}

/*
Principle routine of the scheduler. It gets the thread which be can run from the
runQueue.
*/
void Schedule(void)
{
    kthread* runnable;

    /* Get next thread to run from the run queue */
    disable_interrupts();
    runnable = next_runnable_kthread();
    enable_interrupts();

    /*
     * Activate the new thread, saving the context of the current thread.
     * Eventually, this thread will get re-activated and Switch_To_Thread()
     * will "return", and then Schedule() will return to wherever
     * it was called from.
     */
    if(!runnable)
        PANIC(__FUNCTION__,__LINE__,"Nothing to schedule");
    Switch_To_Thread(runnable);
}


static __inline__ void Push(kthread* k_thread, uint64_t value)
{
    k_thread->rsp -= 0x8;
    *((uint64_t *) k_thread->rsp) = value;
}

static __inline__ void Push_General_Registers(kthread* k_thread){
    /*
     * Push initial values for saved general-purpose registers.
     * (The actual values are not important.)
     */
    Push(k_thread, 0);  /* rax */
    Push(k_thread, 0);  /* rbx */
    Push(k_thread, 0);  /* rcx */
    Push(k_thread, 0);  /* rdx */
    Push(k_thread, 0);  /* rsi */
    Push(k_thread, 0);  /* rdi */
    Push(k_thread, 0);  /* rbp */
    Push(k_thread, 0);  /* r15 */
    Push(k_thread, 0);  /* r14 */
    Push(k_thread, 0);  /* r13 */
    Push(k_thread, 0);  /* r12 */
    Push(k_thread, 0);  /* r11 */
    Push(k_thread, 0);  /* r10 */
    Push(k_thread, 0);  /* r09 */
    Push(k_thread, 0);  /* r08 */

}

/*
Gte the status of interrupts
1<<9 is the bit which corresponds to the interrupt flag in the rflag register
*/
bool is_interrupt_on(void){
    uint64_t rflags = get_flag_register();
    return (rflags & (1<<9));
}

/*
Called when a thread exits. It sets alive to FALSE and calls schedule which will
choose the next thread which can be run.
*/
void Exit_thread(uint16_t exit_code){
    kthread* current = currentThread;

    if(is_interrupt_on())
      disable_interrupts();    

    current->exitCode = exit_code;
    current->alive = FALSE;
    Schedule(); 
}

/*
This is because, while coming out of the scheduler, we come in an interrupt diabled
context. We cannot enable interrupts in the scheduler itself as another interrupt
might possibly fire before even we return from the scheduler.
*/
void Launch_Thread(void)
{
    enable_interrupts();
}

/*
When a thread gracefully exits, we call this function.
*/
static void thread_exit(void)
{
    Exit_thread(0);
}

/*
This is the IDLE thread. If we happen to come here, then we need to just run and not
return from the IDLE thread. Infact we should never return from an Idle thread.
*/
void Idle(uint16_t arg){
    while(TRUE)
      Yield();
}

void Hello(uint16_t arg){
    int i=0;
    while(TRUE){
      i++;
      if(i%1000 == 0)
      printf("Hello ");
    }
    Yield();
}

void World(uint16_t arg){
    int i=0;
    while(TRUE){
      i++;
      if(i%1000 == 0)
        printf("World ");
    }
}

void init_thread_queue(Thr_Queue *node){
    node->head = node->tail = NULL;
}

static void Init_Thread(kthread* k_thread,const char* name, void* stackPage, uint16_t prio, bool detached){
    static int nextFreePid = 0;
    kthread* owner = detached ? (kthread*)0: currentThread;
    k_thread->stackPage = stackPage;
    k_thread->rsp = ((uint64_t) k_thread->stackPage) + VIRT_PAGE_SIZE;
    k_thread->numTicks = 0;
    k_thread->priority = prio;
    //k_thread->userContext = 0;
    k_thread->owner = owner;
    k_thread->refCount = detached ? 1 : 2;

    k_thread->alive = TRUE;
    k_thread->name = name;
    init_thread_queue(&(k_thread->joinQueue));
    k_thread->pid = nextFreePid;
    nextFreePid++;
}

/*
IMPORTANT code for context switching. Logic below.
Why are we pushing so much for the thread which begins. When we context switch and replace the 
current thread by the return value of next runnable thread, everything is new for the incoming thread. Its
true we can directly push the RIP and pop it when the new thread context switches. But what happens
when a thread which was already running context switched and returning back. It needs all its registers back.
So we have to trick the new thread as though it is returning from a previous context switch.
As to why we are pushing, the rflasgs, interrupt number and other extra paramaters is that the same thread
can return from a interrupt context. So we have to prepare the thread for context switch from interrupt context
as well as a normal scheduling context.
*/
void setup_kthread(kthread* k_thread, thread_func startFunc, uint16_t arg){
    // Push arguments
    Push(k_thread, arg);
    // Push the address of exit function
    Push(k_thread, (uint64_t) &thread_exit);
    // Push the address of start function
    Push(k_thread, (uint64_t) startFunc);
    
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
    Push(k_thread, (uint64_t)KERN_DS);

    /* Why +0x8? Because, we need to point to the startFunc once iretq pops. Without 0x8,
    our RSP will be pointing to KERN_DS and it will return to that causing a GPF
    */
    Push(k_thread, (uint64_t)(k_thread->rsp + 0x8));

    // Push rflags
    Push(k_thread, (uint64_t)0);

    // Push kernel CS. Needed for iretq call
    Push(k_thread, (uint64_t)KERN_CS);

    //Push the address of launching function
    Push(k_thread, (uint64_t) &Launch_Thread);

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

/*
A thread has decided to yield for some reason. We need to make it runnable and add it to the
end of the runQueue so that the next time this same thread can be chosen if this thread has the
highest priority of all.
*/
void Yield(void){
    disable_interrupts();
    kthread* current = Get_Current();
    runnable_kthread(current);
    Schedule();
    enable_interrupts(); 
}

/*
Get the thread that is currently running
*/
struct Kernel_Thread* Get_Current(void)
{
    return currentThread;
}

void runnable_kthread(kthread* k_thread){
    append_run_queue(&runQueue, k_thread);
}


static kthread* create_kthread(const char* name, int prio, bool detached){
    kthread* k_thread;
    void* stackPage = 0;
    k_thread = (kthread*)sub_malloc(0,1);
    if( !k_thread )
        return NULL;
    stackPage = (void*)sub_malloc(0,1);
    if( !stackPage ){
        sub_free(stackPage);
        return NULL;
    }
    /*
     * Initialize the stack pointer of the new thread
     * and accounting info
     */
    Init_Thread(k_thread, name, stackPage, prio, detached);

    /* Add to the list of all threads in the system. */
    append_global_list_queue(&allThreadList, k_thread);

    return k_thread;

    
}

void scheduler_init(){
    kthread* main_thread = (kthread*)sub_malloc(0,1);
    void* stackPage = (void*)stack;
    Init_Thread(main_thread,"main",(void *)stackPage, PRIORITY_NORMAL, TRUE);
    currentThread = main_thread;
    append_global_list_queue(&allThreadList, main_thread);
    start_kthread(Idle, "Idle", 0, PRIORITY_IDLE, TRUE);
    start_kthread(start, "Start", 0, PRIORITY_NORMAL, TRUE);
    start_kthread(World, "World", 0, PRIORITY_LOW, TRUE);
    start_kthread(Hello, "Hello", 0, PRIORITY_LOW, TRUE);
    is_scheduler_on = 1;
    Schedule();
}

kthread* start_kthread(thread_func startFunc, const char* name, uint16_t arg, uint16_t priority, bool detached){
    kthread* k_thread = create_kthread(name, priority, detached);
    if(!k_thread)
        return NULL;
    setup_kthread(k_thread, startFunc, arg);
    disable_interrupts();
    runnable_kthread(k_thread);
    enable_interrupts();
    return k_thread;
}