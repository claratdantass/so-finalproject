#include "serial.h"
#include "io.h"

/* ---- port offsets (relative to COM base) ---- */

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  ((base) + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  ((base) + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) ((base) + 4)
#define SERIAL_LINE_STATUS_PORT(base)   ((base) + 5)

/* ---- commands ---- */

#define SERIAL_LINE_ENABLE_DLAB 0x80

/* ---- internal configuration helpers ---- */

/** serial_configure_baud_rate:
 *  Sets the speed of the data being sent. The default speed of a serial
 *  port is 115200 bits/s. The argument is a divisor of that number, hence
 *  the resulting speed becomes (115200 / divisor) bits/s.
 *
 *  @param com      The COM port to configure
 *  @param divisor  The divisor
 */
static void serial_configure_baud_rate(unsigned short com,
                                       unsigned short divisor)
{
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

/** serial_configure_line:
 *  Configures the line of the given serial port. The port is set to have a
 *  data length of 8 bits, no parity bits, one stop bit and break control
 *  disabled.
 *
 *  @param com  The serial port to configure
 */
static void serial_configure_line(unsigned short com)
{
    /* Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     * Content: | d | b | prty  | s | dl  |
     * Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
     */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/** serial_configure_buffers:
 *  Configures the FIFO buffers of the given serial port.
 *  Enables FIFO, clears both queues, uses 14-byte threshold.
 *
 *  @param com  The serial port to configure
 */
static void serial_configure_buffers(unsigned short com)
{
    /* 0xC7 = 1100 0111 */
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

/** serial_configure_modem:
 *  Configures the modem of the given serial port.
 *  Sets RTS and DTR to 1.
 *
 *  @param com  The serial port to configure
 */
static void serial_configure_modem(unsigned short com)
{
    /* 0x03 = 0000 0011 (RTS = 1, DTR = 1) */
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

/* ---- public API ---- */

void serial_init(unsigned short com)
{
    serial_configure_baud_rate(com, 2);   /* 115200 / 2 = 57600 baud */
    serial_configure_line(com);
    serial_configure_buffers(com);
    serial_configure_modem(com);
}

/** serial_is_transmit_fifo_empty:
 *  Checks whether the transmit FIFO queue is empty or not for the given COM
 *  port.
 *
 *  @param  com The COM port
 *  @return 0 if the transmit FIFO queue is not empty
 *          1 if the transmit FIFO queue is empty
 */
static int serial_is_transmit_fifo_empty(unsigned short com)
{
    /* 0x20 = 0010 0000 */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

/** serial_write_char:
 *  Writes a single character to the given serial port.
 *  Spins until the transmit FIFO is empty before sending.
 *
 *  @param com The COM port
 *  @param c   The character to send
 */
static void serial_write_char(unsigned short com, char c)
{
    while (!serial_is_transmit_fifo_empty(com))
        ;

    outb(SERIAL_DATA_PORT(com), c);
}

int serial_write(unsigned short com, const char *buf, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++) {
        serial_write_char(com, buf[i]);
    }

    return (int) len;
}
