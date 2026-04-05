#include "syscall.h"
#include "ulib.h"

static const char *state_name(unsigned int s)
{
    if (s == 1) return "READY  ";
    if (s == 2) return "RUNNING";
    return "DEAD   ";
}

void main(void)
{
    struct proc_info procs[16];
    char cmd[16];
    char numbuf[12];
    int count, i, len, pad;

    while (1) {
        clear();
        write("=== Process Viewer (top) ===\n\n", 30);
        write("PID   STATE    NAME\n", 20);
        write("----  -------  ----------------\n", 31);

        count = getprocs(procs, 16);
        for (i = 0; i < count; i++) {
            itoa((int)procs[i].pid, numbuf);
            write(numbuf, ustrlen(numbuf));

            pad = 6 - (int)ustrlen(numbuf);
            while (pad-- > 0)
                write(" ", 1);

            write(state_name(procs[i].state), 7);
            write("  ", 2);
            write(procs[i].name, ustrlen(procs[i].name));
            write("\n", 1);
        }

        write("\nEnter to refresh, 'q'+Enter to quit.\n", 38);
        len = read(cmd, sizeof(cmd) - 1);
        if (len > 0) {
            cmd[len] = '\0';
            if (cmd[0] == 'q')
                exit();
        }
    }
}
