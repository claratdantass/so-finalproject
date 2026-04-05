#include "pit.h"
#include "io.h"

#define PIT_COMMAND_PORT  0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_BASE_FREQ     1193182U
#define PIC1_DATA_PORT    0x21

void pit_init(unsigned int freq_hz)
{
    unsigned int divisor;
    unsigned char mask;

    if (freq_hz == 0)
        freq_hz = 1;
    divisor = PIT_BASE_FREQ / freq_hz;
    if (divisor > 0xFFFF)
        divisor = 0xFFFF;
    if (divisor == 0)
        divisor = 1;

    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (unsigned char)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (unsigned char)((divisor >> 8) & 0xFF));

    mask = inb(PIC1_DATA_PORT);
    mask &= ~0x01U;
    outb(PIC1_DATA_PORT, mask);
}
