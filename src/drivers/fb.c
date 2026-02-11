#include "fb.h"
#include "io.h"

/* ---- constants ---- */

#define FB_COLS  80
#define FB_ROWS  25
#define FB_SIZE  (FB_COLS * FB_ROWS)  /* total cells */

/* I/O ports for the VGA cursor */
#define FB_COMMAND_PORT      0x3D4
#define FB_DATA_PORT         0x3D5

/* I/O port commands */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND  15

/* default colors */
#define FB_DEFAULT_FG  FB_WHITE
#define FB_DEFAULT_BG  FB_BLACK

/* ---- state ---- */

static char *fb = (char *) 0x000B8000;
static unsigned short fb_cursor = 0;  /* current position in cells (0..1999) */

/* ---- internal helpers ---- */

/** fb_scroll:
 *  Scrolls the framebuffer up by one row.
 *  Copies rows 1..24 to 0..23 and clears the last row.
 */
static void fb_scroll(void)
{
    unsigned int i;

    /* move rows 1..24 up to rows 0..23 (each cell = 2 bytes) */
    for (i = 0; i < (FB_SIZE - FB_COLS) * 2; i++) {
        fb[i] = fb[i + FB_COLS * 2];
    }

    /* clear the last row */
    for (i = (FB_SIZE - FB_COLS) * 2; i < FB_SIZE * 2; i += 2) {
        fb[i]     = ' ';
        fb[i + 1] = ((FB_DEFAULT_FG & 0x0F) << 4) | (FB_DEFAULT_BG & 0x0F);
    }

    fb_cursor = FB_SIZE - FB_COLS;  /* beginning of last row */
}

/* ---- public API ---- */

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    unsigned int offset = i * 2;
    fb[offset]     = c;
    fb[offset + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (pos >> 8) & 0x00FF);
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
}

void fb_clear(void)
{
    unsigned int i;

    for (i = 0; i < FB_SIZE; i++) {
        fb_write_cell(i, ' ', FB_DEFAULT_FG, FB_DEFAULT_BG);
    }

    fb_cursor = 0;
    fb_move_cursor(fb_cursor);
}

int fb_write(const char *buf, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++) {
        char c = buf[i];

        if (c == '\n') {
            /* advance to the start of the next row */
            fb_cursor += FB_COLS - (fb_cursor % FB_COLS);
        } else {
            fb_write_cell(fb_cursor, c, FB_DEFAULT_FG, FB_DEFAULT_BG);
            fb_cursor++;
        }

        /* scroll if we went past the last cell */
        if (fb_cursor >= FB_SIZE) {
            fb_scroll();
        }
    }

    fb_move_cursor(fb_cursor);

    return (int) len;
}
