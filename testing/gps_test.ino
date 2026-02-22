#include <HardwareSerial.h>
#include "../common/config.h"

HardwareSerial GPSSerial(1);

struct GPSData {
    double latitude;
    double longitude;
    float heading;
    uint8_t fix_quality;
    uint8_t num_satellites;
    float hdop;
    float altitude;
    bool valid;
};

GPSData gpsData;

void setup() {
    Serial.begin(115200);
    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    Serial.println("BE-880 GPS/Compass Test");
    Serial.println("Waiting for GPS data...");
}

void parseGPGGA(char* sentence) {
    char* token;
    int fieldNum = 0;
    
    token = strtok(sentence, ",");
    while (token != NULL && fieldNum < 15) {
        switch (fieldNum) {
            case 1:
                if (strlen(token) > 0) {
                    double rawLat = atof(token);
                    int degrees = (int)(rawLat / 100);
                    double minutes = rawLat - (degrees * 100);
                    gpsData.latitude = degrees + (minutes / 60.0);
                }
                break;
            case 2:
                if (token[0] == 'S') gpsData.latitude = -gpsData.latitude;
                break;
            case 3:
                if (strlen(token) > 0) {
                    double rawLon = atof(token);
                    int degrees = (int)(rawLon / 100);
                    double minutes = rawLon - (degrees * 100);
                    gpsData.longitude = degrees + (minutes / 60.0);
                }
                break;
            case 4:
                if (token[0] == 'W') gpsData.longitude = -gpsData.longitude;
                break;
            case 6:
                gpsData.fix_quality = atoi(token);
                break;
            case 7:
                gpsData.num_satellites = atoi(token);
                break;
            case 8:
                gpsData.hdop = atof(token);
                break;
            case 9:
                gpsData.altitude = atof(token);
                break;
        }
        token = strtok(NULL, ",");
        fieldNum++;
    }
    
    gpsData.valid = (gpsData.fix_quality > 0);
}

void parseGPHDT(char* sentence) {
    char* token;
    int fieldNum = 0;
    
    token = strtok(sentence, ",");
    while (token != NULL && fieldNum < 3) {
        if (fieldNum == 1 && strlen(token) > 0) {
            gpsData.heading = atof(token);
        }
        token = strtok(NULL, ",");
        fieldNum++;
    }
}

bool verifyChecksum(char* sentence) {
    char* asterisk = strchr(sentence, '*');
    if (asterisk == NULL) return false;
    
    uint8_t checksum = 0;
    for (char* p = sentence + 1; p < asterisk; p++) {
        checksum ^= *p;
    }
    
    uint8_t receivedChecksum = strtol(asterisk + 1, NULL, 16);
    return (checksum == receivedChecksum);
}

void processNMEA(char* sentence) {
    if (!verifyChecksum(sentence)) {
        Serial.println("Checksum failed!");
        return;
    }
    
    if (strstr(sentence, "$GPGGA") || strstr(sentence, "$GNGGA")) {
        parseGPGGA(sentence);
        
        if (gpsData.valid) {
            Serial.println("\n--- GPS Fix ---");
            Serial.print("Lat: "); Serial.print(gpsData.latitude, 7);
            Serial.print(" Lon: "); Serial.println(gpsData.longitude, 7);
            Serial.print("Fix: "); Serial.print(gpsData.fix_quality);
            Serial.print(" Sats: "); Serial.print(gpsData.num_satellites);
            Serial.print(" HDOP: "); Serial.println(gpsData.hdop, 1);
            Serial.print("Alt: "); Serial.print(gpsData.altitude, 1); Serial.println(" m");
        }
    } else if (strstr(sentence, "$GPHDT") || strstr(sentence, "$GNHDT")) {
        parseGPHDT(sentence);
        Serial.print("Heading: "); Serial.print(gpsData.heading, 1); Serial.println("°");
    }
}

char nmeaBuffer[128];
int bufferIndex = 0;

void loop() {
    while (GPSSerial.available()) {
        char c = GPSSerial.read();
        Serial.write(c); // forward raw GPS byte to USB serial for debugging
        
        if (c == '$') {
            bufferIndex = 0;
            nmeaBuffer[bufferIndex++] = c;
        } else if (c == '\n' || c == '\r') {
            if (bufferIndex > 0) {
                nmeaBuffer[bufferIndex] = '\0';
                processNMEA(nmeaBuffer);
                bufferIndex = 0;
            }
        } else if (bufferIndex < sizeof(nmeaBuffer) - 1) {
            nmeaBuffer[bufferIndex++] = c;
        }
    }
}
