#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define MAX_BUOYS 5
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
    PKT_ASSIGN = 0x01,
    PKT_ACK_ASSIGN = 0x02,
    PKT_STATUS = 0x03,
    PKT_PING_STATUS = 0x04
};

enum BuoyID {
    BUOY_MASTER = 0,
    BUOY_START_A = 1,
    BUOY_START_B = 2,
    BUOY_WINDWARD = 3,
    BUOY_LEEWARD = 4
};

struct GPSPosition {
    double latitude;
    double longitude;
    uint8_t fix_quality;
    float hdop;
};

struct AssignPacket {
    uint8_t packet_type;
    uint8_t buoy_id;
    uint16_t seq;
    double target_lat;
    double target_lon;
    float hold_radius;
    uint32_t timestamp;
    uint8_t checksum;
};

struct AckAssignPacket {
    uint8_t packet_type;
    uint8_t buoy_id;
    uint16_t seq;
    uint8_t accepted;
    double current_lat;
    double current_lon;
    uint8_t checksum;
};

struct StatusPacket {
    uint8_t packet_type;
    uint8_t buoy_id;
    uint8_t state;
    uint8_t gps_fix;
    double current_lat;
    double current_lon;
    float heading;
    float dist_to_target;
    float battery_voltage;
    uint8_t error_flags;
    int8_t rssi;
    int8_t snr;
    uint8_t checksum;
};

struct PingStatusPacket {
    uint8_t packet_type;
    uint8_t buoy_id;
    uint32_t timestamp;
    uint8_t checksum;
};

#define ERROR_FLAG_GPS_LOST     0x01
#define ERROR_FLAG_COMPASS_FAIL 0x02
#define ERROR_FLAG_LOW_BATTERY  0x04
#define ERROR_FLAG_MOTOR_FAIL   0x08
#define ERROR_FLAG_COMMS_LOST   0x10
#define ERROR_FLAG_WIND_FAIL    0x20

#define HOLD_RADIUS_DEFAULT 5.0f
#define COMMS_TIMEOUT_MS 30000
#define GPS_FIX_REQUIRED 3
#define HOLD_TIME_REQUIRED_S 10

#define WIND_CHANGE_THRESHOLD_DEG 12.0f
#define WIND_CHANGE_DURATION_S 30

#define REPLY_DELAY_BASE_MS 100
#define REPLY_DELAY_PER_ID_MS 50

uint8_t calculate_checksum(uint8_t* data, size_t len);
bool verify_checksum(uint8_t* data, size_t len);

#endif
