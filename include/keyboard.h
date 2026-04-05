#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

void keyboard_handler(void);
int  keyboard_has_line(void);
unsigned int keyboard_read_line(char *buf, unsigned int max);

#endif /* INCLUDE_KEYBOARD_H */
