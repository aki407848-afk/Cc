#include "ir_controller.h"
#include "config.h"
#include "hardware.h"
#include <IRsend.h>
#include <WebServer.h>

// === ВАЖНО: Сообщаем компилятору, что server определен в другом файле ===
extern WebServer server;

bool isBruting = false;

void handleIrSend() {
    String type = server.arg("type");
    if(pins.ir_tx == -1) {
        server.send(400, "text/plain", "IR Pin not configured");
        return;
    }
    
    if(type == "lg") {
        irSend.sendNEC(LG_POWER, 32);
        server.send(200, "text/plain", "LG Power Sent");
    } else if(type == "samsung") {
        irSend.sendNEC(SAMSUNG_POWER, 32);
        server.send(200, "text/plain", "Samsung Power Sent");
    } else {
        server.send(400, "text/plain", "Unknown Type");
    }
}

void handleBruteStart() {
    if(pins.ir_tx == -1) {
        server.send(400, "text/plain", "IR Pin not configured");
        return;
    }
    isBruting = true;
    server.send(200, "text/plain", "Brute Started");
}

void handleBruteStop() {
    isBruting = false;
    server.send(200, "text/plain", "Brute Stopped");
}

void runIrBrute() {
    if(!isBruting) return;
    
    static uint32_t code = 0;
    irSend.sendNEC(code, 32);
    code++;
    // Сброс если переполнение (для NEC 32 бита)
    if(code > 0xFFFFFFFF) code = 0;
    
    delay(100); 
}
