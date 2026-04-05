#include "syscall.h"

void main(void)
{
    write("Process B started\n", 18);
    yield();
    write("Process B resumed\n", 18);
    yield();
    write("Process B exiting\n", 18);
    exit();
}
