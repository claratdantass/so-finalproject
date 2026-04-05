#include "syscall.h"
#include "fb.h"
#include "process.h"

extern struct fs_instance rootfs;

void syscall_dispatch(struct syscall_regs *regs)
{
    switch (regs->eax) {
    case SYS_WRITE:
        regs->eax = (unsigned int)fb_write((const char *)regs->ebx, regs->ecx);
        break;
    case SYS_EXIT:
        process_current()->state = PROC_STATE_DEAD;
        schedule();
        while (1) {
            __asm__ __volatile__("hlt");
        }
        break;
    case SYS_YIELD:
        schedule();
        break;
    case SYS_SPAWN:
        regs->eax = (unsigned int)process_create(&rootfs,
                                                  (const char *)regs->ebx);
        break;
    default:
        regs->eax = (unsigned int)-1;
        break;
    }
}
