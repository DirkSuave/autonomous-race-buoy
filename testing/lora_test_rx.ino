#include <SPI.h>
#include <RH_RF95.h>
#include "../common/config.h"

RH_RF95 rf95(LORA_CS_PIN, LORA_IRQ_PIN);

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("LoRa RFM95W Test - Receiver");
    
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
    Serial.println("Waiting for packets...");
}

void loop() {
    if (rf95.available()) {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
        
        if (rf95.recv(buf, &len)) {
            buf[len] = '\0';
            Serial.print("Received: ");
            Serial.println((char*)buf);
            Serial.print("RSSI: ");
            Serial.println(rf95.lastRssi(), DEC);
            Serial.print("SNR: ");
            Serial.println(rf95.lastSNR(), DEC);
            
            char reply[] = "ACK from Slave";
            rf95.send((uint8_t *)reply, strlen(reply));
            rf95.waitPacketSent();
            Serial.println("Sent reply");
        } else {
            Serial.println("Receive failed");
        }
    }
}
