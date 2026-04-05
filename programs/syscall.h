#ifndef PROGRAMS_SYSCALL_H
#define PROGRAMS_SYSCALL_H

#define SYS_WRITE     1
#define SYS_EXIT      2
#define SYS_YIELD     3
#define SYS_SPAWN     4
#define SYS_READ      5
#define SYS_WAIT      6
#define SYS_GETPROCS  7
#define SYS_LISTFILES 8
#define SYS_CLEAR     9

struct proc_info {
    unsigned int pid;
    unsigned int state;
    char name[32];
};

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

static inline void yield(void)
{
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(SYS_YIELD)
    );
}

static inline int spawn(const char *name)
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_SPAWN), "b"(name)
        : "memory"
    );
    return ret;
}

static inline int read(char *buf, unsigned int max)
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_READ), "b"(buf), "c"(max)
        : "memory"
    );
    return ret;
}

static inline int wait(int pid)
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_WAIT), "b"(pid)
        : "memory"
    );
    return ret;
}

static inline int getprocs(struct proc_info *buf, int max)
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_GETPROCS), "b"(buf), "c"(max)
        : "memory"
    );
    return ret;
}

static inline int listfiles(char *buf, unsigned int max)
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_LISTFILES), "b"(buf), "c"(max)
        : "memory"
    );
    return ret;
}

static inline void clear(void)
{
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(SYS_CLEAR)
    );
}

#endif /* PROGRAMS_SYSCALL_H */
