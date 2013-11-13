#include <sys/task.h>
#include <errors.h>
#include <defs.h>
#include <sys/mm/vmmgr_virtual.h>
#include <stdio.h>
#include <stdlib.h>

//task_struct *currentTask;

//static TaskList allTaskList;

//static runQueue_List task_runQueue;

/*void create_pgd(int pid, int orig_pid){
    pml4* pml4_dir = (pml4*)sub_malloc(0,1);
    pml4* pml4_dir_phy = pml4_dir - 0xFFFFFFFF81400000;
}*/

/*
void insert_TaskList(TaskList *list, task_struct* task){
    task->prevTask = NULL;
    if(list->head == NULL){
        list->head = list->tail = task;
        task->nextTask = NULL;
    }
    else{
        task->nextTask = list->head;
        list->head->prevTask = task;
        list->head = task;
    }
}

void append_TaskList(TaskList* list, task_struct* task){
    task->nextTask = NULL;
    if(list->tail == NULL){
        list->head = list->tail = task;
        task->prevTask = NULL;
    }
    else{
          list->tail->nextTask = task;
          task->prevTask = list->tail;
          list->tail = task;
    }
}

void remove_TaskList(TaskList* list, task_struct* task){
    if(task->prevTask != NULL)
        task->prevTask->nextTask = task->nextTask;
    else
        list->head = task->nextTask;
    if(task->nextTask != NULL)
        task->nextTask->prevTask = task->prevTask;
    else
        list->tail = task->prevTask;
}

void insert_runQueue_Task(runQueue_List *list, task_struct* task){
    task->prevRunTask = 0;
    if(list->head == NULL){
        list->head = list->tail = task;
        task->nextRunTask = NULL;
    }
    else{
        task->nextRunTask = list->head;
        list->head->prevRunTask = task;
        list->head = task;
    }
}

void append_runQueue_task(runQueue_List* list, task_struct* task){
    task->nextRunTask = NULL;
    if(list->tail == NULL){
        list->head = list->tail = task;
        task->prevRunTask = NULL;
    }
    else{
          list->tail->nextRunTask = task;
          task->prevRunTask = list->tail;
          list->tail = task;
    }
}


void remove_runQueue_Task(runQueue_List* list, task_struct* task){
    if(task->prevRunTask != NULL)
        task->prevRunTask->nextRunTask = task->nextRunTask;
    else
        list->head = task->nextRunTask;
    if(task->nextRunTask != NULL)
        task->nextRunTask->prevRunTask = task->prevRunTask;
    else
        list->tail = task->prevRunTask;
}

int pid = 0;
pml4* pgd_alloc(){
    pml4* pgd;
    pgd = (pml4*)sub_malloc(sizeof(pml4),1);
    return pgd;
}

void pgd_free(pml4* pgd){
    sub_free(pgd);
}

static inline void mm_free_pgd(task_struct* mm)
{
        pgd_free(mm->pgd);
}
*/
/* static inline mem_error_t mm_alloc_pgd(mm_struct *mm){
    mm->pgd = pgd_alloc(mm);
    if(!mm->pgd){
        return E_MEM_NOMEM;
    }
    return E_MEM_SUCCESS;
}

static mm_struct* mm_init(mm_struct *mm){
    int rc;
    mm->mm_users = 1;
    mm->mm_count = 1;
    rc = mm_alloc_pgd(mm);
    if( rc != E_MEM_SUCCESS ){
        SYS_TRACE(__FUNCTION__,__LINE__,"NoMemory while allocating page tables for user process");
        mm_free_pgd(mm);
    }
    return E_MEM_SUCCESS;
}

mm_struct* mm_alloc(void){
    mm_struct *mm;
    mm = (mm_struct*)sub_malloc(sizeof(mm_struct),0);
    if(!mm){
        SYS_TRACE(__FUNCTION__,__LINE__,"NoMemory while allocating mm_struct for user process");
        sub_free(mm);
    }
    if(mm){
        memset(mm,0,sizeof(mm));
        mm = mm_init(mm);
    }
    return mm;
}
*/
/*
int alloc_pid(){
    return pid++;
}

task_struct* alloc_task_struct(){
    task_struct* ts_t = (task_struct*)sub_malloc(sizeof(struct ts), 0);
    return ts_t;
}

void free_task_struct(task_struct* ts_t){
    sub_free(ts_t);
}

thread_info* alloc_thread_info(task_struct* ts_t){
    thread_info* ti_t = (thread_info*)sub_malloc(sizeof(struct ti), 0);
    return ti_t;
    
}
static task_struct *dup_task_struct(task_struct *orig){
    task_struct *tsk;
    thread_info *ti;
    tsk = alloc_task_struct();
    if(!tsk)
        return NULL;
    ti = alloc_thread_info(tsk);
    if (!ti) {
        free_task_struct(tsk);
        return NULL;
    }
    memcpy((char*)tsk, (const char*)orig, sizeof(struct ts));
    //tsk->cr3 = create_pgd(pid, orig->pid); 
    *tsk = *orig;
    tsk->th_info = ti;
    return tsk;
}

static task_struct* copy_process(uint16_t clone_flags,uint64_t stack_start,regs_t* r,uint32_t stack_size){
    task_struct *p = NULL;
    p = dup_task_struct(currentTask);
    return p; 
} 

int do_fork(uint16_t clone_flags, uint64_t stack_start, regs_t* regs, uint32_t stack_size){
    task_struct *p;
    uint16_t pid = alloc_pid();
    if(pid < 0)
        return E_PID_NOPID;
    p = copy_process(clone_flags, stack_start, regs, stack_size);
    p++;
    return E_PID_SUCCESS;
}
*/
/* 
put_task_struct() to free the pages containing the
process.s kernel stack and thread_info structure and deallocate the slab cache containing
the task_struct.
*/
/*
void put_task_struct(task_struct *tsk){
//    if()
}
*/
