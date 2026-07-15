#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "config_manager.h"
#include "ir_controller.h"
#include "wifi_tools.h"
#include "terminal.h"

// Глобальные объекты
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket для терминала и реального времени

void setup() {
    Serial.begin(115200);
    
    // 1. Инициализация файловой системы
    if(!LittleFS.begin(true)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
    
    // 2. Загрузка конфига
    loadConfig();
    
    // 3. Настройка WiFi (AP Mode по умолчанию для доступа к сайту)
    String apSsid = "ESP32-Toolbox-" + String(WiFi.macAddress());
    apSsid.replace(":", "");
    WiFi.softAP(apSsid.c_str(), "admin123");
    
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // 4. Инициализация модулей на основе пинов из конфига
    initModulesFromConfig();

    // 5. Настройка Web Server
    setupWebServer();
    
    // 6. Настройка WebSocket (Терминал)
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.begin();
    Serial.println("Server started");
}

void loop() {
    // Обработка веб-сокета
    ws.cleanupClients();
    
    // Если есть команды из терминала (через WS), обрабатываем их
    processTerminalCommands();
    
    // Фоновые задачи модулей (если нужно)
    yield();
}
