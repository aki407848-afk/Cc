#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "config.h"
#include "hardware.h"
#include "web_server.h"
#include "ir_controller.h" // Убедись, что тут есть прототип runIrBrute()
#include "file_manager.h"

// Прототип функции спама из web_server.cpp
extern void runWifiSpamTask();
extern bool isSpamming; // Если нужно проверять статус

PinConfig pins;
String apSsid = DEFAULT_AP_SSID;
String apPass = DEFAULT_AP_PASS;

void setup() {
    Serial.begin(115200);
    
    if(!LittleFS.begin(true)){
        Serial.println("LittleFS Mount Failed");
    }

    loadConfig();
    initHardware();

    WiFi.softAP(apSsid.c_str(), apPass.c_str());
    Serial.print("AP Started: ");
    Serial.println(WiFi.softAPIP());

    startWebServer();

    Serial.println("System Ready.");
}

void loop() {
    server.handleClient();
    handleTerminalCommands(); 
    handleLocalButtons();     
    
    // Запуск фоновых задач
    runIrBrute();      // Из ir_controller.cpp
    runWifiSpamTask(); // Из web_server.cpp
}
