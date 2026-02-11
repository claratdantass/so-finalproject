#ifndef INCLUDE_FB_H
#define INCLUDE_FB_H

/* VGA text-mode color constants (4-bit values) */
#define FB_BLACK         0
#define FB_BLUE          1
#define FB_GREEN         2
#define FB_CYAN          3
#define FB_RED           4
#define FB_MAGENTA       5
#define FB_BROWN         6
#define FB_LIGHT_GREY    7
#define FB_DARK_GREY     8
#define FB_LIGHT_BLUE    9
#define FB_LIGHT_GREEN  10
#define FB_LIGHT_CYAN   11
#define FB_LIGHT_RED    12
#define FB_LIGHT_MAGENTA 13
#define FB_LIGHT_BROWN  14
#define FB_WHITE        15

/** fb_clear:
 *  Clears the entire framebuffer (fills with spaces) and resets the cursor
 *  to position (0,0).
 */
void fb_clear(void);

/** fb_write_cell:
 *  Writes a character with the given foreground and background to position i
 *  in the framebuffer (cell index, not byte offset).
 *
 *  @param i  The cell index (0 .. 80*25 - 1)
 *  @param c  The character
 *  @param fg The foreground color
 *  @param bg The background color
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);

/** fb_move_cursor:
 *  Moves the cursor of the framebuffer to the given position.
 *
 *  @param pos The new position of the cursor (cell index)
 */
void fb_move_cursor(unsigned short pos);

/** fb_write:
 *  Writes the contents of the buffer buf of length len to the screen.
 *  Automatically advances the cursor and scrolls when necessary.
 *
 *  @param buf The buffer to write
 *  @param len Number of bytes to write
 *  @return    The number of bytes written
 */
int fb_write(const char *buf, unsigned int len);

#endif /* INCLUDE_FB_H */
