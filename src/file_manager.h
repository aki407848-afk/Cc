#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#include <WebServer.h>

void handleFileList();
void handleFileDel();
void handleFileCreate();
void handleTerminalCmd();
void handleTerminalCommands(); // Для Serial
void executeJsonFile(String filename);

#endif
