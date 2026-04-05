/* Mini-shell: loop readline → parse; built-ins locais ou spawn + wait. */

#include "syscall.h"
#include "ulib.h"

void main(void)
{
    char cmd[128];
    char filebuf[512];
    int len, pid;

    /* ---- Banner ---- */
    write("=== MiniShell v1.0 ===\n", 23);
    write("Type 'help' for available commands.\n\n", 37);

    /* ---- REPL: prompt, normalização da linha, dispatch ---- */
    while (1) {
        write("> ", 2);
        len = read(cmd, sizeof(cmd) - 1);

        if (len <= 0)
            continue;
        cmd[len] = '\0';

        while (len > 0 && (cmd[len - 1] == ' ' || cmd[len - 1] == '\n'))
            cmd[--len] = '\0';

        if (len == 0)
            continue;

        /* Built-ins */
        if (streq(cmd, "help")) {
            write("Available commands:\n", 20);
            write("  help    - show this help\n", 27);
            write("  ls      - list programs\n", 26);
            write("  clear   - clear screen\n", 25);
            write("  exit    - halt the system\n", 28);
            write("  <name>  - run a program\n", 26);
        } else if (streq(cmd, "ls")) {
            len = listfiles(filebuf, sizeof(filebuf));
            if (len > 0)
                write(filebuf, (unsigned int)len);
        } else if (streq(cmd, "clear")) {
            clear();
        } else if (streq(cmd, "exit")) {
            write("Goodbye!\n", 9);
            exit();
        } else {
            /* Programa no SOFS */
            pid = spawn(cmd);
            if (pid < 0) {
                write("Unknown command: ", 17);
                write(cmd, ustrlen(cmd));
                write("\n", 1);
            } else {
                wait(pid);
            }
        }
    }
}
