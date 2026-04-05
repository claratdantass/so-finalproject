#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include "fs.h"

#define PROC_STATE_DEAD    0
#define PROC_STATE_READY   1
#define PROC_STATE_RUNNING 2

#define MAX_PROCESSES 16

struct process {
    unsigned int pid;
    unsigned int state;
    unsigned int esp;
    unsigned int page_dir_phys;
    unsigned int kernel_stack;
    unsigned int kernel_stack_top;
};

void process_init(void);
int  process_create(struct fs_instance *fs, const char *name);
void schedule(void);
void scheduler_start(void);
struct process *process_current(void);

#endif /* INCLUDE_PROCESS_H */
