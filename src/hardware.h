#ifndef HARDWARE_H
#define HARDWARE_H
#include "config.h"
#include <IRsend.h>

extern IRsend irSend;

void loadConfig();
void saveConfig();
void initHardware();
void handleLocalButtons();

#endif
