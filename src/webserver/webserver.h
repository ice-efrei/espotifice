#ifndef WEB_SERVER_H
#define WEB_SERVER_H
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "../../User_settings.h"

void handleRoot(AsyncWebServerRequest *request, String local_ip);
int handleCallbackPage(
    AsyncWebServerRequest *request,
    int (*validationFunction)(String code_param),
    String local_ip);
void setup_web_server(AsyncWebServer *server, int (*validationFunction)(String code_param));

#endif