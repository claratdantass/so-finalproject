#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

#define SERIAL_COM1_BASE 0x3F8

/** serial_init:
 *  Initializes the given serial port: configures baud rate, line, buffers
 *  and modem.
 *
 *  @param com The base address of the COM port (e.g. SERIAL_COM1_BASE)
 */
void serial_init(unsigned short com);

/** serial_write:
 *  Writes the contents of the buffer buf of length len to the given serial
 *  port. Waits for the transmit FIFO to be empty before sending each byte.
 *
 *  @param com The base address of the COM port
 *  @param buf The buffer to write
 *  @param len Number of bytes to write
 *  @return    The number of bytes written
 */
int serial_write(unsigned short com, const char *buf, unsigned int len);

#endif /* INCLUDE_SERIAL_H */
