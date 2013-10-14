#include <sys/task.h>
#include <errors.h>
#include <defs.h>
#include <sys/mm/vmmgr_virtual.h>
#include <stdio.h>
#include <stdlib.h>

task_struct *current;

int pid = 0;
pml4* pgd_alloc(){
    pml4* pgd;
    pgd = (pml4*)sub_malloc(sizeof(pml4));
    return pgd;
}

void pgd_free(pml4* pgd){
    sub_free(pgd);
}

static inline void mm_free_pgd(mm_struct* mm)
{
        pgd_free(mm->pgd);
}

static inline mem_error_t mm_alloc_pgd(mm_struct *mm){
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
    mm = (mm_struct*)sub_malloc(sizeof(mm_struct));
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

int alloc_pid(){
    return pid++;
}

task_struct* alloc_task_struct(){
    task_struct* ts_t = (task_struct*)sub_malloc(sizeof(struct ts));
    return ts_t;
}

void free_task_struct(task_struct* ts_t){
    sub_free(ts_t);
}

thread_info* alloc_thread_info(task_struct* ts_t){
    thread_info* ti_t = (thread_info*)sub_malloc(sizeof(struct ti));
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
    *tsk = *orig;
    tsk->thread_info = ti;
    return tsk;
}

static task_struct* copy_process(uint16_t clone_flags,uint64_t stack_start,regs_t* r,uint32_t stack_size){
    task_struct *p = NULL;
    p = dup_task_struct(current);
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
/* 
put_task_struct() to free the pages containing the
process.s kernel stack and thread_info structure and deallocate the slab cache containing
the task_struct.
*/
void put_task_struct(task_struct *tsk){
//    if()
}
