#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include "fs.h"

#define PROC_STATE_DEAD    0
#define PROC_STATE_READY   1
#define PROC_STATE_RUNNING 2

#define MAX_PROCESSES 16
#define PROC_NAME_MAX 32

struct process {
    unsigned int pid;
    unsigned int state;
    unsigned int esp;
    unsigned int page_dir_phys;
    unsigned int kernel_stack;
    unsigned int kernel_stack_top;
    char name[PROC_NAME_MAX];
};

struct proc_info {
    unsigned int pid;
    unsigned int state;
    char name[PROC_NAME_MAX];
};

void process_init(void);
int  process_create(struct fs_instance *fs, const char *name);
void schedule(void);
void scheduler_start(void);
struct process *process_current(void);
int  process_is_alive(unsigned int pid);
int  process_get_info(struct proc_info *buf, int max);

#endif /* INCLUDE_PROCESS_H */
