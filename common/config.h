#ifndef CONFIG_H
#define CONFIG_H

#define LORA_CS_PIN     5
#define LORA_RST_PIN    14
#define LORA_IRQ_PIN    2
#define LORA_FREQ       915.0

#define GPS_RX_PIN      16
#define GPS_TX_PIN      17
#define GPS_BAUD        115200

#define WIND_SPEED_PIN  34
#define WIND_DIR_PIN    35

#define MOTOR_LEFT_PWM  25
#define MOTOR_LEFT_DIR  26
#define MOTOR_RIGHT_PWM 27
#define MOTOR_RIGHT_DIR 33

#define BATTERY_ADC_PIN 36
#define VOLTAGE_DIVIDER_RATIO 11.0f

#define LOOP_RATE_HZ 10
#define STATUS_REPORT_INTERVAL_MS 5000

#endif
