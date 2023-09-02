#include "./webserver.h"

int temp_code_validator(String code_param)
{
    return 0;
}

void setup_web_server(AsyncWebServer *server, int (*validationFunction)(String code_param))
{
    server->on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello from ESP32 server route");
    });

    server->on("/",
              HTTP_GET, [](AsyncWebServerRequest *request)
              { handleRoot(request, WiFi.localIP().toString()); });

    server->on("/callback",
              HTTP_GET, [validationFunction](AsyncWebServerRequest *request)
              { handleCallbackPage(request, validationFunction, WiFi.localIP().toString()); });

    Serial.println("Web server started");
    server->begin();
}

void handleRoot(AsyncWebServerRequest *request, String local_ip)
{
    String redirect_url = AUTHENTICATION_URL +
                          "?response_type=code" +
                          "&client_id=" + CLIENT_ID +
                          "&scope=" + SPOTIFY_SCOPE +
                          "&redirect_uri=http://" + local_ip + "/callback" +
                          "&show_dialog=login";
    request->redirect(redirect_url);
}

int handleCallbackPage(
    AsyncWebServerRequest *request,
    int (*validationFunction)(String code_param),
    String local_ip)
{
    String redirect_url = AUTHENTICATION_URL +
                          "?response_type=code" +
                          "&client_id=" + CLIENT_ID +
                          "&scope=" + SPOTIFY_SCOPE +
                          "&redirect_uri=http://" + local_ip + "/callback";

    if (request->arg("code") == "")
    {
        redirect_url += "&show_dialog=error";
        request->redirect(redirect_url);
        return 1;
    }

    String code_param = request->arg("code");
    int validation_result = validationFunction(code_param);

    if (validation_result) // != 0
    {
        redirect_url += "&show_dialog=error&error=" + String(validation_result);
        request->redirect(redirect_url);
        return 2;
    }

    redirect_url += "&show_dialog=success";
    request->redirect(redirect_url);
    return 0;
}