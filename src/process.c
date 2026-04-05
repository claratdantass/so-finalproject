#include "process.h"
#include "pfa.h"
#include "paging4k.h"
#include "kutil.h"
#include "tss.h"
#include "fb.h"

#define PAGE_SIZE 4096U
#define USER_CODE_VIRT  0x00000000U
#define USER_STACK_VIRT 0xBFFFF000U
#define USER_STACK_TOP  0xC0000000U

extern void switch_context(unsigned int *old_esp, unsigned int new_esp,
                           unsigned int new_cr3);
extern void jump_to_process(unsigned int saved_esp, unsigned int page_dir_phys);
extern void process_start_trampoline(void);

static struct process procs[MAX_PROCESSES];
static struct process *current;
static unsigned int next_pid = 1;

void process_init(void)
{
    unsigned int i;

    for (i = 0; i < MAX_PROCESSES; i++) {
        procs[i].state = PROC_STATE_DEAD;
        procs[i].pid = 0;
    }
    current = (struct process *)0;
}

struct process *process_current(void)
{
    return current;
}

int process_create(struct fs_instance *fs, const char *name)
{
    int slot, fs_idx;
    unsigned int prog_data, prog_size, code_pages;
    unsigned int *user_dir;
    unsigned int stack_frame, code_frame;
    unsigned int kstack_base, kstack_top;
    unsigned int offset, copy_len, i;
    unsigned int *sp;

    for (slot = 0; slot < MAX_PROCESSES; slot++) {
        if (procs[slot].state == PROC_STATE_DEAD)
            break;
    }
    if (slot == MAX_PROCESSES)
        return -1;

    fs_idx = fs_find(fs, name);
    if (fs_idx < 0)
        return -1;

    prog_data = fs_file_data(fs, fs_idx);
    prog_size = fs_file_size(fs, fs_idx);
    code_pages = (prog_size + PAGE_SIZE - 1U) / PAGE_SIZE;

    kstack_base = pfa_alloc();
    if (kstack_base == 0U)
        return -1;
    memset((void *)kstack_base, 0, PAGE_SIZE);
    kstack_top = kstack_base + PAGE_SIZE;

    user_dir = paging_create_kernel_dir();
    if (user_dir == (unsigned int *)0)
        return -1;

    for (i = 0; i < code_pages; i++) {
        code_frame = pfa_alloc();
        if (code_frame == 0U)
            return -1;
        offset = i * PAGE_SIZE;
        copy_len = prog_size - offset;
        if (copy_len > PAGE_SIZE)
            copy_len = PAGE_SIZE;
        memcpy((void *)code_frame, (void *)(prog_data + offset), copy_len);
        if (copy_len < PAGE_SIZE)
            memset((void *)(code_frame + copy_len), 0, PAGE_SIZE - copy_len);
        paging_map_4k(user_dir, USER_CODE_VIRT + offset, code_frame,
                       PTE_PRESENT | PTE_RW | PTE_USER);
    }

    stack_frame = pfa_alloc();
    if (stack_frame == 0U)
        return -1;
    memset((void *)stack_frame, 0, PAGE_SIZE);
    paging_map_4k(user_dir, USER_STACK_VIRT, stack_frame,
                   PTE_PRESENT | PTE_RW | PTE_USER);

    /*
     * Fake kernel stack: looks like switch_context just saved state
     * and the process is about to enter user mode via the trampoline.
     *
     * High address (kernel_stack_top):
     *   SS_user  (0x23)
     *   ESP_user
     *   EFLAGS   (0x0202, IF=1)          <- iret frame
     *   CS_user  (0x1B)
     *   EIP      (entry point)
     *   &process_start_trampoline        <- ret target for switch_context
     *   EBP (0)                          <- switch_context restore frame
     *   EBX (0)
     *   ESI (0)
     *   EDI (0)
     *   EFLAGS (0x0002, IF=0)            <- for popfd (interrupts off until iret)
     * Low address:  <- process->esp
     */
    sp = (unsigned int *)kstack_top;

    *(--sp) = 0x23;
    *(--sp) = USER_STACK_TOP;
    *(--sp) = 0x0202;
    *(--sp) = 0x1B;
    *(--sp) = USER_CODE_VIRT;

    *(--sp) = (unsigned int)process_start_trampoline;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0x0002;

    procs[slot].pid = next_pid++;
    procs[slot].state = PROC_STATE_READY;
    procs[slot].esp = (unsigned int)sp;
    procs[slot].page_dir_phys = (unsigned int)user_dir;
    procs[slot].kernel_stack = kstack_base;
    procs[slot].kernel_stack_top = kstack_top;

    return (int)procs[slot].pid;
}

void schedule(void)
{
    struct process *old;
    struct process *next = (struct process *)0;
    unsigned int i, start;

    if (!current)
        return;

    start = 0;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (&procs[i] == current) {
            start = i;
            break;
        }
    }

    for (i = 1; i <= MAX_PROCESSES; i++) {
        unsigned int idx = (start + i) % MAX_PROCESSES;
        if (procs[idx].state == PROC_STATE_READY) {
            next = &procs[idx];
            break;
        }
    }

    if (!next)
        return;

    old = current;
    if (old->state == PROC_STATE_RUNNING)
        old->state = PROC_STATE_READY;
    next->state = PROC_STATE_RUNNING;
    current = next;

    tss_set_kernel_stack(next->kernel_stack_top);
    switch_context(&old->esp, next->esp, next->page_dir_phys);
}

void scheduler_start(void)
{
    unsigned int i;
    struct process *first = (struct process *)0;

    for (i = 0; i < MAX_PROCESSES; i++) {
        if (procs[i].state == PROC_STATE_READY) {
            first = &procs[i];
            break;
        }
    }

    if (!first) {
        fb_write("No processes to run.\n", 21);
        return;
    }

    first->state = PROC_STATE_RUNNING;
    current = first;
    tss_set_kernel_stack(first->kernel_stack_top);
    jump_to_process(first->esp, first->page_dir_phys);
}
