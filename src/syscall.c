/* Despacho de syscalls: int 0x80 chega aqui via syscall_handler.s.
 * Cada caso traduz a chamada do usuário para funções do kernel. */

#include "syscall.h"
#include "fb.h"
#include "fs.h"
#include "keyboard.h"
#include "process.h"

extern struct fs_instance rootfs;

void syscall_dispatch(struct syscall_regs *regs)
{
    switch (regs->eax) {
    /* ---- Saída no console (VGA) ---- */
    case SYS_WRITE:
        regs->eax = (unsigned int)fb_write((const char *)regs->ebx, regs->ecx);
        break;

    /* ---- Ciclo de vida e agendamento de processos ---- */
    case SYS_EXIT:
        process_current()->state = PROC_STATE_DEAD;
        schedule();
        while (1) {
            __asm__ __volatile__("hlt");
        }
        break;

    case SYS_YIELD:
        schedule();
        break;

    case SYS_SPAWN:
        regs->eax = (unsigned int)process_create(&rootfs,
                                                  (const char *)regs->ebx);
        break;

    /* ---- Entrada e sincronização (bloqueio cooperativo + hlt) ---- */
    case SYS_READ:
        while (!keyboard_has_line()) {
            schedule();
            if (!keyboard_has_line()) {
                __asm__ __volatile__("sti; hlt; cli");
            }
        }
        regs->eax = keyboard_read_line((char *)regs->ebx, regs->ecx);
        break;

    case SYS_WAIT: {
        unsigned int wait_pid = regs->ebx;
        while (process_is_alive(wait_pid)) {
            schedule();
            if (process_is_alive(wait_pid)) {
                __asm__ __volatile__("sti; hlt; cli");
            }
        }
        regs->eax = 0;
        break;
    }

    /* ---- Introspecção: tabela de processos e listagem SOFS ---- */
    case SYS_GETPROCS:
        regs->eax = (unsigned int)process_get_info(
            (struct proc_info *)regs->ebx, (int)regs->ecx);
        break;

    case SYS_LISTFILES:
        regs->eax = (unsigned int)fs_list(&rootfs,
                                           (char *)regs->ebx, regs->ecx);
        break;

    /* ---- Limpeza de tela ---- */
    case SYS_CLEAR:
        fb_clear();
        regs->eax = 0;
        break;

    /* ---- Syscall desconhecida ---- */
    default:
        regs->eax = (unsigned int)-1;
        break;
    }
}
