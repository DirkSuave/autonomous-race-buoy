#include "../common/config.h"

volatile uint32_t windSpeedPulseCount = 0;
volatile uint32_t lastWindSpeedPulse = 0;
uint32_t lastWindSpeedCalc = 0;

float windSpeed = 0.0;
float windDirection = 0.0;

void IRAM_ATTR windSpeedISR() {
    uint32_t now = millis();
    if (now - lastWindSpeedPulse > 10) {
        windSpeedPulseCount++;
        lastWindSpeedPulse = now;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Davis Vantage Pro Wind Sensor Test");
    
    pinMode(WIND_SPEED_PIN, INPUT_PULLUP);
    pinMode(WIND_DIR_PIN, INPUT);
    
    attachInterrupt(digitalPinToInterrupt(WIND_SPEED_PIN), windSpeedISR, FALLING);
    
    lastWindSpeedCalc = millis();
    
    Serial.println("Calibration Notes:");
    Serial.println("- Wind speed: 1 pulse per second = 1 mph");
    Serial.println("- Wind direction: 0-3.3V analog, calibrate to 0-360°");
    Serial.println();
}

float calculateWindSpeed() {
    uint32_t now = millis();
    uint32_t elapsed = now - lastWindSpeedCalc;
    
    if (elapsed >= 1000) {
        float pulsesPerSecond = (float)windSpeedPulseCount / (elapsed / 1000.0);
        windSpeed = pulsesPerSecond * 1.609344;
        
        windSpeedPulseCount = 0;
        lastWindSpeedCalc = now;
    }
    
    return windSpeed;
}

float calculateWindDirection() {
    int rawValue = analogRead(WIND_DIR_PIN);
    float voltage = (rawValue / 4095.0) * 3.3;
    
    windDirection = (voltage / 3.3) * 360.0;
    
    return windDirection;
}

void loop() {
    float speed = calculateWindSpeed();
    float direction = calculateWindDirection();
    
    Serial.print("Wind Speed: ");
    Serial.print(speed, 2);
    Serial.print(" km/h (");
    Serial.print(speed / 1.852, 2);
    Serial.print(" knots) | Direction: ");
    Serial.print(direction, 1);
    Serial.print("° | Raw ADC: ");
    Serial.println(analogRead(WIND_DIR_PIN));
    
    delay(500);
}
