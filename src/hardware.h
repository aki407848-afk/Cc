#ifndef HARDWARE_H
#define HARDWARE_H

#include "config.h"
#include <IRsend.h>

// Объявляем глобальный объект IRsend, чтобы он был виден везде
extern IRsend irSend;

void loadConfig();
void saveConfig();
void initHardware();
void handleLocalButtons();

#endif
