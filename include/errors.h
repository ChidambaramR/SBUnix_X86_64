#ifndef _ERRORS_H
#define _ERROR_H

enum _memory_error{
    E_MEM_SUCCESS = 0,
    E_MEM_NOMEM = -1, // Out of memoru
};
typedef enum _memory_error mem_error_t;

enum _task_error{
    E_PID_SUCCESS = 0,
    E_PID_NOPID = -1,
};
typedef enum _task_error task_error_t;

#endif
