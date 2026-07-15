#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "config.h"
#include "hardware.h"
#include "web_server.h"
#include "ir_controller.h"
#include "file_manager.h"

PinConfig pins;
String apSsid = DEFAULT_AP_SSID;
String apPass = DEFAULT_AP_PASS;

void setup() {
    Serial.begin(115200);
    
    // 1. Файловая система
    if(!LittleFS.begin(true)){
        Serial.println("LittleFS Mount Failed");
    } else {
        Serial.println("LittleFS Mounted");
    }

    // 2. Загрузка конфига
    loadConfig();

    // 3. Инициализация железа (ИК, Экран, Кнопки)
    initHardware();

    // 4. Запуск WiFi AP
    WiFi.softAP(apSsid.c_str(), apPass.c_str());
    Serial.print("AP Started: ");
    Serial.println(WiFi.softAPIP());

    // 5. Запуск Веб-сервера
    startWebServer();

    Serial.println("System Ready.");
}

void loop() {
    server.handleClient();
    handleTerminalCommands(); // Проверка команд из Serial
    handleLocalButtons();     // Обработка локальных кнопок, если есть
}
