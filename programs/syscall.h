#ifndef PROGRAMS_SYSCALL_H
#define PROGRAMS_SYSCALL_H

#define SYS_WRITE 1
#define SYS_EXIT  2

static inline int write(const char *buf, unsigned int len)
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_WRITE), "b"(buf), "c"(len)
        : "memory"
    );
    return ret;
}

static inline void exit(void)
{
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(SYS_EXIT)
    );
    while (1) {}
}

#endif /* PROGRAMS_SYSCALL_H */
