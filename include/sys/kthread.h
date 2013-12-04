#ifndef _KTHREAD_H_
#define _KTHREAD_H_

#include <sys/task.h>

/*
 * Thread priorities
 */
#define PRIORITY_IDLE    0
#define PRIORITY_USER    1
#define PRIORITY_LOW     2
#define PRIORITY_NORMAL  5
#define PRIORITY_HIGH   10

/*
 * Thread start functions should have this signature.
 */
typedef void (*thread_func)(uint16_t arg);
typedef struct Kernel_Thread kthread;

struct All_threads{
  kthread *head, *tail;
};
typedef struct All_threads global_thread_list;

struct Thread_queue{
    kthread *head, *tail;
};
typedef struct Thread_queue Thr_Queue;



struct join_Queue{
  kthread* child;
  struct join_Queue* next;
};
typedef struct join_Queue joinQ;
/*
 * Kernel thread context data structure.
 * NOTE: there is assembly code in lowlevel.asm that depends
 * on the offsets of the fields in this struct, so if you change
 * the layout, make sure everything gets updated.
 */
struct Kernel_Thread {
    uint64_t rsp;                         /* offset 0 */
    uint64_t kernel_thread;
    uint64_t krsp;
    uint64_t pcr3;
    uint64_t cr3;
    void* kstack;
    signed int sleeping;           /* offset 4 */
    int priority;
    struct Kernel_Thread *prev_in_ThreadQ, *next_in_ThreadQ;
    void* stackPage;
    int brk;
    int no_stack_pages;
//    struct User_Context* userContext;
    struct Kernel_Thread* owner;
    struct Kernel_Thread* parent;
    struct Thread_queue joinQueue;
    int refCount;
    joinQ head;
    /* These fields are used to implement the Join() function */
    bool alive;
    const char* name;
//    struct Thread_Queue joinQueue;
    int exitCode;

    /* The kernel thread id; also used as process id */
    int pid;
    vm_area_struct *mmap;
    vm_area_struct *mmap_cache;
    /* Link fields for list of all threads in the system. */
    struct Kernel_Thread *prev_in_ThreadList, *next_in_ThreadList;
    /* Array of MAX_TLOCAL_KEYS pointers to thread-local data. */
#define MAX_TLOCAL_KEYS 128
    const void* tlocalData[MAX_TLOCAL_KEYS];

};
typedef struct Kernel_Thread kthread;


/*
 * Scheduler operations.
 */
void Init_Scheduler(void);

void setup_kthread(kthread*, thread_func, uint16_t);
void Enqueue_Thread(Thr_Queue*, kthread*);
void Dequeue_Thread(Thr_Queue*, kthread*);
kthread* start_kthread(thread_func, const char*, uint16_t, uint16_t, bool);
//kthread* start_uthread(struct User_Context* userContext, bool detached);
void runnable_kthread(kthread*);
kthread* Get_Current(void);
kthread* Get_Next_Runnable(void);
void Schedule(void);
void scheduler_init(void);
void Yield(void);
void Exit_thread(uint16_t exitCode); // __attribute__ ((noreturn));
int Join(struct Kernel_Thread* kthread);
kthread* Lookup_Thread(int pid);
void disable_interrupts();
void enable_interrupts();
void append_run_queue(Thr_Queue*, kthread*);
void append_global_list_queue(global_thread_list*, kthread*);
void Push_General_Registers(kthread*);
void Push(kthread*, uint64_t);
void init_thread_queue(Thr_Queue*);
int alloc_pid();
void free_pid();
void add_to_ptable(kthread*);
void thread_cleanup(kthread*);
void alllist_kthread(kthread*);
void add_to_joinQueue(kthread*, kthread*);
void remove_runnable_kthread(Thr_Queue*, kthread*);
void* tarfs_read(char*);
void main_execve(char*);
 void remove_alllist_kthread(global_thread_list*, kthread*);
#endif
