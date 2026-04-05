/* Driver de teclado: IRQ1 lê scan codes, ecoa no VGA e monta uma linha
 * em buffer até Enter; o syscall SYS_READ consome essa linha. */

#include "io.h"
#include "fb.h"
#include "keyboard.h"

#define KBD_DATA_PORT   0x60
#define KB_LINE_MAX     128

/* ---- Tabela scan code (set 1) → ASCII ---- */
static const char scancode_to_ascii[128] = {
    0,   27, '1', '2', '3', '4', '5', '6',
   '7', '8', '9', '0', '-', '=','\b','\t',
   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
   'o', 'p', '[', ']','\n',  0,  'a', 's',
   'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
  '\'', '`',  0, '\\', 'z', 'x', 'c', 'v',
   'b', 'n', 'm', ',', '.', '/',  0,  '*',
    0,  ' ',  0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

/* ---- Estado da linha atual (até SYS_READ esvaziar) ---- */
static char kb_line_buf[KB_LINE_MAX];
static unsigned int kb_line_len = 0;
static volatile int kb_line_complete = 0;

static unsigned char read_scan_code(void)
{
    return inb(KBD_DATA_PORT);
}

/* ---- Tratamento de IRQ: eco + backspace + fim de linha ---- */
void keyboard_handler(void)
{
    unsigned char scan_code = read_scan_code();
    char c;

    if (scan_code & 0x80)
        return;

    c = scancode_to_ascii[scan_code];
    if (c == 0)
        return;

    if (kb_line_complete)
        return;

    if (c == '\n') {
        kb_line_complete = 1;
        fb_write("\n", 1);
    } else if (c == '\b') {
        if (kb_line_len > 0) {
            kb_line_len--;
            fb_write("\b", 1);
        }
    } else {
        if (kb_line_len < KB_LINE_MAX - 1) {
            kb_line_buf[kb_line_len++] = c;
            fb_write(&c, 1);
        }
    }
}

/* ---- API usada pelo syscall SYS_READ ---- */
int keyboard_has_line(void)
{
    return kb_line_complete;
}

unsigned int keyboard_read_line(char *buf, unsigned int max)
{
    unsigned int i, len;

    len = kb_line_len;
    if (len > max)
        len = max;
    for (i = 0; i < len; i++)
        buf[i] = kb_line_buf[i];

    kb_line_len = 0;
    kb_line_complete = 0;
    return len;
}
