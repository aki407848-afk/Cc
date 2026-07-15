#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === ПИНЫ ПО УМОЛЧАНИЮ (-1 = ОТКЛЮЧЕНО) ===
struct PinConfig {
    int oled_sda = -1;
    int oled_scl = -1;
    int ir_tx = -1;
    int btn_1 = -1;
    int btn_2 = -1;
    int btn_3 = -1;
    int nrf_ce = -1;
    int nrf_csn = -1;
};

extern PinConfig pins;

// === WIFI AP ===
const char* AP_SSID = "ESP32-C3-Control";
const char* AP_PASS = "12345678";

// === IR CODES ===
#define SAMSUNG_POWER 0xE0E040BF
#define LG_POWER 0x20DF10EF

#endif
