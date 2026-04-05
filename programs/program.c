#include "syscall.h"

void main(void)
{
    write("Process A started\n", 18);
    yield();
    write("Process A resumed\n", 18);
    yield();
    write("Process A exiting\n", 18);
    exit();
}
