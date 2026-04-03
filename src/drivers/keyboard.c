/* Keyboard driver: reads scan codes from the keyboard data port
 * and translates them to ASCII using a US QWERTY layout table.
 * Only handles key presses (ignores key releases with bit 7 set).
 */

#include "io.h"
#include "fb.h"
#include "keyboard.h"

#define KBD_DATA_PORT   0x60

/* US QWERTY scan code to ASCII lookup (set 1, lowercase only)
 * Index = scan code, value = ASCII char (0 = unmapped) */
static const char scancode_to_ascii[128] = {
    0,   27, '1', '2', '3', '4', '5', '6',     /* 0x00 - 0x07 */
   '7', '8', '9', '0', '-', '=','\b','\t',     /* 0x08 - 0x0F */
   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',     /* 0x10 - 0x17 */
   'o', 'p', '[', ']','\n',  0,  'a', 's',     /* 0x18 - 0x1F */
   'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 0x20 - 0x27 */
  '\'', '`',  0, '\\', 'z', 'x', 'c', 'v',     /* 0x28 - 0x2F */
   'b', 'n', 'm', ',', '.', '/',  0,  '*',     /* 0x30 - 0x37 */
    0,  ' ',  0,   0,   0,   0,   0,   0,       /* 0x38 - 0x3F */
    0,   0,   0,   0,   0,   0,   0,   0,       /* 0x40 - 0x47 */
    0,   0,   0,   0,   0,   0,   0,   0,       /* 0x48 - 0x4F */
    0,   0,   0,   0,   0,   0,   0,   0,       /* 0x50 - 0x57 */
    0,   0,   0,   0,   0,   0,   0,   0,       /* 0x58 - 0x5F */
    0,   0,   0,   0,   0,   0,   0,   0,       /* 0x60 - 0x67 */
    0,   0,   0,   0,   0,   0,   0,   0,       /* 0x68 - 0x6F */
    0,   0,   0,   0,   0,   0,   0,   0,       /* 0x70 - 0x77 */
    0,   0,   0,   0,   0,   0,   0,   0        /* 0x78 - 0x7F */
};

/* Read a scan code from the keyboard data port */
static unsigned char read_scan_code(void)
{
    return inb(KBD_DATA_PORT);
}

/* Handle keyboard IRQ: read scan code, translate, display on screen */
void keyboard_handler(void)
{
    unsigned char scan_code = read_scan_code();

    /* Ignore key releases (bit 7 set) */
    if (scan_code & 0x80) {
        return;
    }

    char c = scancode_to_ascii[scan_code];
    if (c != 0) {
        fb_write(&c, 1);
    }
}
