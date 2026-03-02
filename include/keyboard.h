/* Keyboard driver - reads scan codes and translates to ASCII */

#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

/* Called from the interrupt dispatcher when IRQ1 fires */
void keyboard_handler(void);

#endif /* INCLUDE_KEYBOARD_H */
