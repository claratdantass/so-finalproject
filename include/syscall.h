#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#define SYS_WRITE 1
#define SYS_EXIT  2

struct syscall_regs {
    unsigned int ebp;
    unsigned int edi;
    unsigned int esi;
    unsigned int edx;
    unsigned int ecx;
    unsigned int ebx;
    unsigned int eax;
};

void syscall_dispatch(struct syscall_regs *regs);

#endif /* INCLUDE_SYSCALL_H */
