#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include "common/config.h"

RH_RF95 rf95(LORA_CS_PIN, LORA_IRQ_PIN);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) delay(10);

    Serial.println("LoRa RFM95W Test - Transmitter");

    pinMode(LORA_RST_PIN, OUTPUT);
    digitalWrite(LORA_RST_PIN, HIGH);
    delay(10);
    digitalWrite(LORA_RST_PIN, LOW);
    delay(10);
    digitalWrite(LORA_RST_PIN, HIGH);
    delay(10);

    if (!rf95.init()) {
        Serial.println("LoRa init failed!");
        while (1);
    }

    if (!rf95.setFrequency(LORA_FREQ)) {
        Serial.println("setFrequency failed!");
        while (1);
    }

    rf95.setTxPower(23, false);
    rf95.setSpreadingFactor(7);
    rf95.setSignalBandwidth(125000);
    rf95.setCodingRate4(5);

    Serial.print("LoRa initialized at ");
    Serial.print(LORA_FREQ);
    Serial.println(" MHz");
}

uint16_t packetNum = 0;

void loop() {
    char radiopacket[50];
    sprintf(radiopacket, "Packet #%d - Test from Master", packetNum++);

    Serial.print("Sending: ");
    Serial.println(radiopacket);

    rf95.send((uint8_t *)radiopacket, strlen(radiopacket));
    rf95.waitPacketSent();

    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.waitAvailableTimeout(3000)) {
        if (rf95.recv(buf, &len)) {
            buf[len] = '\0';
            Serial.print("Received: ");
            Serial.println((char*)buf);
            Serial.print("RSSI: ");
            Serial.println(rf95.lastRssi(), DEC);
            Serial.print("SNR: ");
            Serial.println(rf95.lastSNR(), DEC);
        } else {
            Serial.println("Receive failed");
        }
    } else {
        Serial.println("No reply, is receiver running?");
    }

    delay(2000);
}
