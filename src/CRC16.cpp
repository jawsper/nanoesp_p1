#include "CRC16.h"

unsigned int CRC16(unsigned int crc, unsigned char *buf, unsigned int len)
{
    for (unsigned int pos = 0; pos < len; pos++)
    {
        crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) {    // Loop over each bit
            if ((crc & 0x0001) != 0) {      // If the LSB is set
                crc >>= 1;                    // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else                            // Else LSB is not set
                crc >>= 1;                    // Just shift right
        }
    }

    return crc;
}

unsigned int CRC16(unsigned int crc, const String &buf)
{
    return CRC16(crc, (unsigned char*)buf.c_str(), buf.length());
}

