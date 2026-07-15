#include "file_manager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "hardware.h"
#include <WebServer.h>

extern WebServer server;

int currentTheme = 1; 

// === ПРОТОТИПЫ ФУНКЦИЙ (чтобы компилятор видел их до вызова) ===
String processCommand(String cmd);
void executeJsonFile(String filename);

// === WEB HANDLERS ===

void handleFileList() {
    String json = "[";
    bool first = true;
    
    // Современный способ перебора файлов в LittleFS
    File root = LittleFS.open("/");
    if(!root.isDirectory()){
        server.send(500, "text/plain", "FS Error");
        return;
    }
    
    File file = root.openNextFile();
    while(file){
        if(!first) json += ",";
        // file.name() возвращает полный путь, нам нужно только имя
        String fname = String(file.name());
        if(fname.startsWith("/")) fname.remove(0, 1);
        
        json += "{\"name\":\"" + fname + "\",\"size\":" + String(file.size()) + "}";
        first = false;
        file = root.openNextFile();
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

// === ЛОГИКА КОМАНД ===

String processCommand(String cmd) {
    if(cmd.startsWith("checks")) {
        String res = "Modules:\n";
        res += "IR: " + String(pins.ir_tx == -1 ? "Disabled" : "Enabled on "+String(pins.ir_tx)) + "\n";
        res += "OLED: " + String(pins.oled_sda == -1 ? "Disabled" : "Enabled") + "\n";
        res += "NRF: " + String(pins.nrf_ce == -1 ? "Disabled" : "Enabled") + "\n";
        return res;
    }
    
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

    if(cmd.startsWith("create json")) {
        return "Use Web Interface for file creation.";
    }

    if(cmd.startsWith("active")) {
        String fname = cmd.substring(7);
        fname.trim();
        if(fname.length() > 0) {
            executeJsonFile("/" + fname);
            return "Executing " + fname;
        }
        return "Usage: active filename.json";
    }

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
    
    // === ИСПРАВЛЕНИЕ ARDUINOJSON V7 ===
    JsonDocument doc; 
    
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    
    if(err) {
        Serial.println("JSON Parse Error: " + String(err.c_str()));
        return;
    }
    
    if(doc["ir_send"].is<uint32_t>()) {
        uint32_t code = doc["ir_send"];
        irSend.sendNEC(code, 32);
        Serial.println("IR Code sent from JSON: 0x" + String(code, HEX));
    } else {
        Serial.println("No 'ir_send' key in JSON or wrong type.");
    }
}
