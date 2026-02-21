#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define MAX_BUOYS 6
#define LORA_MAX_PAYLOAD 251

enum BuoyState {
    STATE_INIT = 0,
    STATE_DEPLOY,
    STATE_HOLD,
    STATE_ADJUST,
    STATE_RECOVER,
    STATE_FAILSAFE
};

enum PacketType {
    PKT_ASSIGN      = 0xA5,  // Master → slave: target coordinates
    PKT_ACK_ASSIGN  = 0xAA,  // Slave → master: assignment acknowledged
    PKT_STATUS      = 0x5A,  // Slave → master: position/battery telemetry
    PKT_PING_STATUS = 0x55,  // Heartbeat
    PKT_RC_START    = 0xB1,  // Remote control → master: start race
    PKT_RC_STOP     = 0xB2   // Remote control → master: stop/abort race
};

enum BuoyID {
    BUOY_MASTER   = 0,
    BUOY_START_A  = 1,
    BUOY_START_B  = 2,
    BUOY_WINDWARD = 3,
    BUOY_LEEWARD  = 4,
    BUOY_REMOTE   = 5   // Handheld remote control unit (two identical units, same ID)
};

struct GPSPosition {
    float   latitude;
    float   longitude;
    uint8_t fix_quality;
    float   hdop;
};

// Master → slave: assign target position (13 bytes)
struct __attribute__((packed)) AssignPacket {
    uint8_t  packet_type;   // PKT_ASSIGN (0xA5)
    uint8_t  buoy_id;
    float    target_lat;
    float    target_lon;
    uint8_t  hold_radius;   // metres
    uint16_t checksum;      // CRC16-CCITT
};

// Slave → master: confirm receipt of assignment (13 bytes)
struct __attribute__((packed)) AckAssignPacket {
    uint8_t  packet_type;   // PKT_ACK_ASSIGN (0xAA)
    uint8_t  buoy_id;
    uint8_t  accepted;      // 1 = accepted, 0 = rejected
    float    current_lat;
    float    current_lon;
    uint16_t checksum;      // CRC16-CCITT
};

// Slave → master: position and battery telemetry (15 bytes)
struct __attribute__((packed)) StatusPacket {
    uint8_t  packet_type;       // PKT_STATUS (0x5A)
    uint8_t  buoy_id;
    float    current_lat;
    float    current_lon;
    uint16_t dist_to_target_cm; // Distance to assigned target in centimetres
    uint8_t  battery_tenths_v;  // Battery voltage in 0.1V units (e.g. 148 = 14.8V)
    uint16_t checksum;          // CRC16-CCITT
};

// Heartbeat (8 bytes)
struct __attribute__((packed)) PingStatusPacket {
    uint8_t  packet_type;   // PKT_PING_STATUS (0x55)
    uint8_t  buoy_id;
    uint32_t timestamp;     // millis()
    uint16_t checksum;      // CRC16-CCITT
};

// Remote control → master: start or stop race (4 bytes)
struct __attribute__((packed)) RcCommandPacket {
    uint8_t  packet_type;   // PKT_RC_START (0xB1) or PKT_RC_STOP (0xB2)
    uint8_t  buoy_id;       // BUOY_REMOTE
    uint16_t checksum;      // CRC16-CCITT
};

// Error flags (StatusPacket.error_flags if extended packet added in future)
#define ERROR_FLAG_GPS_LOST     0x01
#define ERROR_FLAG_COMPASS_FAIL 0x02
#define ERROR_FLAG_LOW_BATTERY  0x04
#define ERROR_FLAG_MOTOR_FAIL   0x08
#define ERROR_FLAG_COMMS_LOST   0x10
#define ERROR_FLAG_WIND_FAIL    0x20

// Operational constants
#define HOLD_RADIUS_DEFAULT         3       // metres (default slave hold radius)
#define COMMS_TIMEOUT_MS            60000   // 60s without ASSIGN → slave enters failsafe
#define GPS_FIX_REQUIRED            3       // Minimum fix quality (3D fix)
#define HOLD_TIME_REQUIRED_S        10      // Seconds on-station before HOLD confirmed

// Master wind stability constants
#define WIND_CHANGE_THRESHOLD_DEG   15.0f   // Max wind shift before Repositioning
#define WIND_CHANGE_DURATION_S      60      // Rolling window for stability check

// LoRa reply timing (deterministic stagger to avoid collisions)
#define REPLY_DELAY_BASE_MS         100
#define REPLY_DELAY_PER_ID_MS       50

uint16_t calculate_checksum(uint8_t* data, size_t len);
bool     verify_checksum(uint8_t* data, size_t len);

#endif // PROTOCOL_H
