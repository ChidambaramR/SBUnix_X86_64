#include <stdio.h>
#include <sys/mm/mmgr.h>
#include <sys/mm/vmmgr_virtual.h>
#include <defs.h>

#define SLOB_UNIT sizeof(slob_t)
#define SLOB_UNITS(size) (((size) + SLOB_UNIT - 1) / SLOB_UNIT)
#define SLOB_ALIGN 8
#define __ALIGN_KERNEL(x, a) __ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a) __ALIGN_KERNEL((x), (a))

struct slob_block{
    int units;
    struct slob_block *next;
};
typedef struct slob_block slob_t;

struct bigblock{
    int order;
    void *pages;
    struct bigblock *next;
};
typedef struct bigblock bigblock_t;

static slob_t arena = { .next = &arena, .units = 1};
static slob_t *slobfree = &arena;
//static bigblock_t *bigblocks;

void slob_free(void *block, int size){
    slob_t* cur, *b = (slob_t*)block;
    if(!block)
        return; 
    if(size)
        b->units = SLOB_UNITS(size);
    
    for(cur = slobfree; !((b > cur) && (b < cur->next)); cur = cur->next){
        if(cur >= cur->next && (b > cur || b < cur->next))
            break;
    }
  
    if (b + b->units == cur->next) {
        b->units += cur->next->units;
        b->next = cur->next->next;
    } 
    else
        b->next = cur->next;

    if (cur + cur->units == b) {
      cur->units += b->units;
      cur->next = b->next;
    } 
    else
        cur->next = b;

    slobfree = cur;
}

void *slob_alloc(int size, int align){
    slob_t *prev, *cur, *aligned = 0;
    int delta = 0;
    int units = SLOB_UNITS(size);
    //unsigned long flags;
    prev = slobfree;
    for(cur = prev->next; ; prev = cur, cur = cur->next){
        if(align){
            aligned = (slob_t *)ALIGN((uint64_t)cur, align);
            delta = aligned - cur; 
        }
        if(cur->units >= units + delta){
            if(delta){
                aligned->units = cur->units - delta;
                aligned->next = cur->next;
                cur->next = aligned;
                cur->units = delta;
                prev = cur;
                cur = aligned;
            }
            if(cur->units == units)
                prev->next = cur->next;
            else{
                prev->next = cur + units;
                prev->next->units = cur->units - units;
                prev->next->next = cur->next;
                cur->units = units;
            }
        
            slobfree = prev;
            return cur;
        }
        if(cur == slobfree){
            if(size == 4096)
                return 0;
            cur = (slob_t*)vmmgr_alloc_page(0);
            slob_free(cur,4096);
            if(!cur)
                return 0;
            
        }
    }
}

