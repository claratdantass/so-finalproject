#include "syscall.h"

void main(void)
{
    write("Hello from user mode!\n", 22);
    exit();
}
