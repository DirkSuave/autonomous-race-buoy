#include "protocol.h"

// CRC16-CCITT (polynomial 0x1021, initial value 0xFFFF)
// Covers all bytes except the final 2-byte checksum field.
uint16_t calculate_checksum(uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Verify checksum: compute CRC over all bytes except last 2, compare to stored value.
// Assumes the stored uint16_t checksum is the last 2 bytes of the packet (little-endian).
bool verify_checksum(uint8_t* data, size_t len) {
    if (len < 3) return false;
    uint16_t computed = calculate_checksum(data, len - 2);
    uint16_t stored   = (uint16_t)data[len - 2] | ((uint16_t)data[len - 1] << 8);
    return computed == stored;
}
