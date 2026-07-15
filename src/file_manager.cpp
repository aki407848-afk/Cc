#include "file_manager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"

// Глобальная переменная для цвета темы (хранится в RAM, можно сохранить в конфиг)
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

// Обработка команд из Serial Monitor
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
        // В веб-терминале это делается через форму, в Serial это сложно реализовать без буфера.
        // Для простоты в Serial: create json test.json {"key":1}
        return "Use Web Interface for file creation or implement buffer parsing.";
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
    if(!LittleFS.exists(filename)) return;
    File f = LittleFS.open(filename, "r");
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    
    if(err) {
        Serial.println("JSON Parse Error");
        return;
    }
    
    // Пример логики: если в JSON есть ключ "ir_send", отправить код
    if(doc.containsKey("ir_send")) {
        uint32_t code = doc["ir_send"];
        irSend.sendNEC(code, 32);
        Serial.println("IR Code sent from JSON: " + String(code, HEX));
    }
    
    // Можно добавить логику для WiFi, пинов и т.д.
}
