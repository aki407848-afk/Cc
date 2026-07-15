#include "file_manager.h"
#include <LittleFS.h>
#include <ArduinoJson.h> // В v7 это включает всё нужное
#include "config.h"
#include "hardware.h"    // Подключаем hardware.h, чтобы видеть irSend
#include <WebServer.h>   // Нужен для доступа к объекту server, если он не extern

// Внешняя ссылка на веб-сервер (он определен в web_server.cpp)
extern WebServer server;

// Глобальная переменная для цвета темы
int currentTheme = 1; 

void handleFileList() {
    Dir dir = LittleFS.openDir("/");
    String json = "[";
    bool first = true;
    while(dir.next()) {
        if(!first) json += ",";
        json += "{\"name\":\"" + dir.fileName() + "\",\"size\":" + String(dir.fileSize()) + "}";
        first = false;
    }
    json += "]";
    server.send(200, "application/json", json);
}

void handleFileDel() {
    String name = "/" + server.arg("name");
    if(LittleFS.exists(name)) {
        LittleFS.remove(name);
        server.send(200, "text/plain", "Deleted");
    } else {
        server.send(404, "text/plain", "Not Found");
    }
}

void handleFileCreate() {
    String name = "/" + server.arg("name");
    String content = server.arg("content");
    File f = LittleFS.open(name, "w");
    f.print(content);
    f.close();
    server.send(200, "text/plain", "Created");
}

void handleTerminalCmd() {
    String cmd = server.arg("cmd");
    String response = processCommand(cmd);
    server.send(200, "text/plain", response);
}

void handleTerminalCommands() {
    if(Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if(cmd.length() > 0) {
            String resp = processCommand(cmd);
            Serial.println(resp);
        }
    }
}

String processCommand(String cmd) {
    // checks
    if(cmd.startsWith("checks")) {
        String res = "Modules:\n";
        res += "IR: " + String(pins.ir_tx == -1 ? "Disabled" : "Enabled on "+String(pins.ir_tx)) + "\n";
        res += "OLED: " + String(pins.oled_sda == -1 ? "Disabled" : "Enabled") + "\n";
        res += "NRF: " + String(pins.nrf_ce == -1 ? "Disabled" : "Enabled") + "\n";
        return res;
    }
    
    // color
    if(cmd.startsWith("color")) {
        if(cmd.length() > 6) {
            int t = cmd.substring(6).toInt();
            if(t>=1 && t<=3) {
                currentTheme = t;
                return "Theme set to " + String(t);
            }
        }
        return "Usage: color [1-3]";
    }

    // create json [name]
    if(cmd.startsWith("create json")) {
        return "Use Web Interface for file creation.";
    }

    // active [name]
    if(cmd.startsWith("active")) {
        String fname = cmd.substring(7);
        fname.trim();
        if(fname.length() > 0) {
            executeJsonFile("/" + fname);
            return "Executing " + fname;
        }
        return "Usage: active filename.json";
    }

    // del [name]
    if(cmd.startsWith("del")) {
        String fname = cmd.substring(4);
        fname.trim();
        if(LittleFS.exists("/" + fname)) {
            LittleFS.remove("/" + fname);
            return "Deleted " + fname;
        }
        return "File not found";
    }

    return "Unknown Command";
}

void executeJsonFile(String filename) {
    if(!LittleFS.exists(filename)) {
        Serial.println("File not found: " + filename);
        return;
    }
    
    File f = LittleFS.open(filename, "r");
    
    // === ИСПРАВЛЕНИЕ ДЛЯ ARDUINOJSON V7 ===
    // Вместо StaticJsonDocument<1024> используем просто JsonDocument
    JsonDocument doc; 
    
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    
    if(err) {
        Serial.println("JSON Parse Error: " + String(err.c_str()));
        return;
    }
    
    // Проверка наличия ключа в стиле v7
    if(doc["ir_send"].is<uint32_t>()) {
        uint32_t code = doc["ir_send"];
        // irSend теперь виден благодаря #include "hardware.h" и extern в нем
        irSend.sendNEC(code, 32);
        Serial.println("IR Code sent from JSON: 0x" + String(code, HEX));
    } else {
        Serial.println("No 'ir_send' key in JSON or wrong type.");
    }
}
