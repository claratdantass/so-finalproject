/* Programa de demonstração carregável pelo shell. */

#include "syscall.h"

void main(void)
{
    write("Hello, World!\n", 14);
    exit();
}
