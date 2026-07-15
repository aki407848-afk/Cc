#ifndef WEB_SERVER_H
#define WEB_SERVER_H
#include <WebServer.h>

extern WebServer server;

void startWebServer();
String getIndexHtml(); // Возвращает HTML страницу

#endif
