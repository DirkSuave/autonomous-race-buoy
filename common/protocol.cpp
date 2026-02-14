#include "protocol.h"

uint8_t calculate_checksum(uint8_t* data, size_t len) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

bool verify_checksum(uint8_t* data, size_t len) {
    if (len < 1) return false;
    uint8_t received_checksum = data[len - 1];
    uint8_t calculated_checksum = calculate_checksum(data, len - 1);
    return (received_checksum == calculated_checksum);
}
