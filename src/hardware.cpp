#include "hardware.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <IRsend.h>

IRsend irSend(0); 

void loadConfig() {
    if(LittleFS.exists("/config.json")) {
        File file = LittleFS.open("/config.json", "r");
        JsonDocument doc; 
        
        DeserializationError error = deserializeJson(doc, file);
        if (!error) {
            pins.oled_sda = doc["oled_sda"] | -1;
            pins.oled_scl = doc["oled_scl"] | -1;
            pins.ir_tx = doc["ir_tx"] | -1;
            pins.btn_1 = doc["btn_1"] | -1;
            pins.btn_2 = doc["btn_2"] | -1;
            pins.btn_3 = doc["btn_3"] | -1;
            pins.nrf_ce = doc["nrf_ce"] | -1;
            pins.nrf_csn = doc["nrf_csn"] | -1;
            
            // === ИСПРАВЛЕНИЕ V7: Проверяем тип вместо containsKey ===
            if(doc["ap_ssid"].is<const char*>()) apSsid = doc["ap_ssid"].as<String>();
            if(doc["ap_pass"].is<const char*>()) apPass = doc["ap_pass"].as<String>();
        }
        file.close();
    }
}

void saveConfig() {
    File file = LittleFS.open("/config.json", "w");
    JsonDocument doc; 
    
    doc["oled_sda"] = pins.oled_sda;
    doc["oled_scl"] = pins.oled_scl;
    doc["ir_tx"] = pins.ir_tx;
    doc["btn_1"] = pins.btn_1;
    doc["btn_2"] = pins.btn_2;
    doc["btn_3"] = pins.btn_3;
    doc["nrf_ce"] = pins.nrf_ce;
    doc["nrf_csn"] = pins.nrf_csn;
    doc["ap_ssid"] = apSsid;
    doc["ap_pass"] = apPass;
    
    serializeJson(doc, file);
    file.close();
}

void initHardware() {
    if(pins.ir_tx != -1) {
        irSend = IRsend(pins.ir_tx);
        irSend.begin();
        Serial.println("IR initialized on pin " + String(pins.ir_tx));
    }
    
    if(pins.btn_1 != -1) pinMode(pins.btn_1, INPUT_PULLUP);
    if(pins.btn_2 != -1) pinMode(pins.btn_2, INPUT_PULLUP);
    if(pins.btn_3 != -1) pinMode(pins.btn_3, INPUT_PULLUP);
}

void handleLocalButtons() {
    if(pins.btn_1 != -1 && digitalRead(pins.btn_1) == LOW) {
        delay(200);
    }
}
