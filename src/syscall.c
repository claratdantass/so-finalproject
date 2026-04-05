#include "syscall.h"
#include "fb.h"

void syscall_dispatch(struct syscall_regs *regs)
{
    switch (regs->eax) {
    case SYS_WRITE:
        regs->eax = (unsigned int)fb_write((const char *)regs->ebx, regs->ecx);
        break;
    case SYS_EXIT:
        while (1) {
            __asm__ __volatile__("hlt");
        }
        break;
    default:
        regs->eax = (unsigned int)-1;
        break;
    }
}
