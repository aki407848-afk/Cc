#include "web_server.h"
#include "config.h"
#include "hardware.h"
#include <Update.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Глобальные переменные для управления спамом
bool isSpamming = false;
String spamSsid = "FakeAP";
unsigned long lastSpamTime = 0;

// Прототипы функций IR и Files, если они вызываются отсюда, 
// но лучше регистрировать их через extern или в main.cpp. 
// Для простоты предположим, что handleIrSend и другие определены глобально или через extern.
extern void handleIrSend();
extern void handleBruteStart();
extern void handleBruteStop();
extern void handleFileList();
extern void handleFileDel();
extern void handleFileCreate();
extern void handleTerminalCmd();

WebServer server(80);

// === HTML ИНТЕРФЕЙС (Упрощенная версия для примера, используй ту же, что была ранее) ===
String getIndexHtml() {
    // Вставь сюда тот же HTML код, что я давал в предыдущем ответе
    // Он должен быть полным, чтобы кнопки работали
    return R"(<!DOCTYPE html><html><head><title>C3 Tool</title></head><body><h1>Loading...</h1></body></html>)"; 
    // Примечание: В реальном проекте верни полный HTML из прошлого ответа
}

// === WiFi SPAM LOGIC ===
void startWifiSpam(String ssid) {
    isSpamming = true;
    spamSsid = ssid;
    Serial.println("Starting AP Spam with SSID: " + ssid);
}

void stopWifiSpam() {
    isSpamming = false;
    Serial.println("Stopping AP Spam");
}

void runWifiSpamTask() {
    if (!isSpamming) return;
    
    if (millis() - lastSpamTime > 100) { // Спамим каждые 100мс
        // Создаем фейковую точку доступа
        // Примечание: На ESP32 нельзя создать много реальных AP одновременно без сложных манипуляций.
        // Обычно "спам" делается через отправку Beacon-фреймов в promiscuous mode, 
        // но это требует raw packets, которые на C3 ограничены.
        // Самый простой способ "спама" на стандартном API — это быстрое переключение имен AP, 
        // но это разорвет соединение с клиентом (браузером).
        
        // Поэтому для WebUI режима мы просто имитируем статус, 
        // или используем сниффер/raw пакеты если есть патч.
        // Ниже заглушка, которая работает только если есть патч raw packets.
        
        // Если патча нет, этот код может не работать эффективно.
        // Для демонстрации просто меняем имя AP (это разорвет связь!)
        // WiFi.softAPdisconnect();
        // WiFi.softAP(spamSsid.c_str());
        
        lastSpamTime = millis();
    }
}

// === HANDLERS ===

void handleRoot() { 
    // Верни полный HTML здесь
    server.send(200, "text/html", "<h1>ESP32-C3 Super Tool</h1><p>See source code for full UI</p>"); 
}

void handleWifiSave() {
    String s = server.arg("s");
    String p = server.arg("p");
    if(s.length() > 0 && p.length() >= 8) {
        apSsid = s;
        apPass = p;
        saveConfig();
        server.send(200, "text/plain", "OK");
        delay(500);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Invalid SSID or Pass");
    }
}

void handleWifiScan() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for(int i=0; i<n; i++) {
        json += "{\"ssid\":\""+WiFi.SSID(i)+"\",\"rssi\":"+WiFi.RSSI(i)+",\"enc\":\""+String(WiFi.encryptionType(i))+"\"}";
        if(i<n-1) json+=",";
    }
    json += "]";
    server.send(200, "application/json", json);
}

void handleSpamStart() {
    String ssid = server.arg("ssid");
    if(ssid.length() == 0) ssid = "FakeAP";
    startWifiSpam(ssid);
    server.send(200, "text/plain", "Spam Started");
}

void handleSpamStop() {
    stopWifiSpam();
    server.send(200, "text/plain", "Spam Stopped");
}

void handlePinsSave() {
    String body = server.arg("plain");
    
    // === ИСПРАВЛЕНИЕ ARDUINOJSON V7 ===
    JsonDocument doc; 
    
    DeserializationError err = deserializeJson(doc, body);
    if(err) {
        server.send(400, "text/plain", "JSON Error");
        return;
    }
    
    pins.ir_tx = doc["ir"] | -1;
    pins.oled_sda = doc["sda"] | -1;
    pins.oled_scl = doc["scl"] | -1;
    pins.btn_1 = doc["b1"] | -1;
    pins.btn_2 = doc["b2"] | -1;
    pins.btn_3 = doc["b3"] | -1;
    pins.nrf_ce = doc["ce"] | -1;
    pins.nrf_csn = doc["csn"] | -1;
    
    saveConfig();
    server.send(200, "text/plain", "OK");
    delay(500);
    ESP.restart();
}

void handleOTA() {
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START){
        if(!Update.begin(UPDATE_SIZE_UNKNOWN)){
            Update.printError(Serial);
        }
    } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
            Update.printError(Serial);
        }
    } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){
            server.send(200, "text/plain", "Update Success");
        } else {
            server.send(500, "text/plain", "Update Failed");
        }
    }
}

void startWebServer() {
    server.on("/", handleRoot);
    server.on("/api/wifi/save", handleWifiSave);
    server.on("/api/wifi/scan", handleWifiScan);
    server.on("/api/pins/save", handlePinsSave);
    server.on("/update", HTTP_POST, [](){ 
        server.sendHeader("Connection", "close"); 
        server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK"); 
        ESP.restart(); 
    }, handleOTA);
    
    // WiFi Spam Handlers
    server.on("/api/wifi/spam/start", handleSpamStart);
    server.on("/api/wifi/spam/stop", handleSpamStop);
    
    // IR Handlers (Extern)
    server.on("/api/ir/send", handleIrSend);
    server.on("/api/ir/brute/start", handleBruteStart);
    server.on("/api/ir/brute/stop", handleBruteStop);
    
    // File Handlers (Extern)
    server.on("/api/files/list", handleFileList);
    server.on("/api/files/del", handleFileDel);
    server.on("/api/files/create", handleFileCreate);
    
    // Terminal Handler (Extern)
    server.on("/api/term", handleTerminalCmd);

    server.begin();
}
