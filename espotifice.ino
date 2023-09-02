/**
 * ESPotifICE
 * An ICE EFREI Project
 * Brief : ESP32 Spotify display
 * Long Functional Description : This project aims to display the current song playing on a Spotify account on a TFT display.
 */
// Base
#include <WiFi.h>

// Spotify API handling
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <base64.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

// TFT display
#include <TFT_eSPI.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
// #include <PNGdec.h>

// Local
#include "src/spotifyapi/spotifyapi.h"
#include "src/webserver/webserver.h"
#include "src/tftdisplay/tftdisplay.h"
// #include "assets/panda.h"

// Server web local
AsyncWebServer server(80); // Server on port 80

// Spotify API
String token, refresh_token;
SongDetails song;
CurrentTrackDetails current_track;
String previous_song_id = "";
int last_refresh = 0; const int refresh_interval = 1000 * 60 * 1; // 5 minutes
int progress_ms = 0, start_time = 0;
bool is_thumbnail_loaded = false;
uint8_t *thumbnail;
int iDataSize;
int validationFunction(String code_param)
{
    return clientCodeValidator(code_param, &token, &refresh_token);
}

// IO pins
#define IO_PINS true
#define VOLUME_PIN 36
int previous_volume = -1;
int map_volume(int potentio_value){
    return map(potentio_value, 0, 4095, 0, 100);
}

// TFT display
TFT_eSPI tft = TFT_eSPI(SCREEN_WIDTH, SCREEN_HEIGHT); // Invoke custom library
int state = 0, last_state_ms = 0;
bool error_refreshing = false;
int thumbnail_x = SCREEN_WIDTH/2 - IMAGE_SIZE/2;
int thumbnail_y = 20;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

int wifi_setup(TFT_eSPI *tft){
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // WiFi connection
    int trials = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
        wifi_setup_tft_display(tft, trials++);
    }

    clear_tft_display(tft);
    display_wifi_connected(tft);
    clear_tft_display(tft);
    Serial.println(WiFi.localIP());
    
    return 0;
}

void setup()
{
    Serial.begin(115200);

    // TFT display setup
    setup_tft_display(&tft, &tft_output);

    // WiFi setup
    wifi_setup(&tft);

    // Web Server setup
    setup_web_server(&server, &validationFunction);

   // Spotify API setup
    spotify_api_setup(
        &tft, 
        &token,
        &refresh_token,
        &current_track,
        &song,
        &progress_ms,
        &start_time,
        &last_refresh,
        &last_state_ms);
}


void loop(){
    if (last_state_ms + 300 < millis()){
        last_state_ms = millis();
        state = (state + 1) % 255; // arbitrary value, just to avoid too big values
        display_current_song(&tft, &current_track, state);
    }

    if (
        last_refresh + refresh_interval < millis() || 
        previous_song_id == "" || 
        progress_ms >= current_track.item->duration_ms
        ){
        if (getCurrentSong(&current_track, &token, &refresh_token)){
            Serial.println("Error refreshing current song");
            clear_tft_display(&tft);
            display_error_tft_display(&tft, "Error refreshing current song");
            delay(5000);
            error_refreshing = true;
            return;
        }
    }

    if (current_track.item->id != previous_song_id)
    {
        Serial.println("New song detected");
        previous_song_id = current_track.item->id;
        is_thumbnail_loaded = false;
        progress_ms = current_track.progress_ms; start_time = millis() - progress_ms;
        last_refresh = millis();
        last_state_ms = millis();
        state = 0;

        clear_tft_display(&tft);
        display_current_song(&tft, &current_track, state);
    }

    if (!is_thumbnail_loaded){
        Serial.println("Loading thumbnail");
        getFileToArray(current_track.item->album_image, &thumbnail, &iDataSize); // TODO : handle error
        display_album_image(&tft, thumbnail, iDataSize, thumbnail_x, thumbnail_y, IMAGE_SIZE, IMAGE_SIZE);
        is_thumbnail_loaded = true;
    }

    if (error_refreshing) {
        Serial.println("Error refreshing");
        clear_tft_display(&tft);
        display_current_song(&tft, &current_track, state);
        display_album_image(&tft, thumbnail, iDataSize, thumbnail_x, thumbnail_y, IMAGE_SIZE, IMAGE_SIZE);
        error_refreshing = false;
        progress_ms = current_track.progress_ms; start_time = millis() - progress_ms;
        last_refresh = millis() + refresh_interval/2; // refresh faster due to error
    }

    if (Serial.available()) { // if there is data comming
        String command = Serial.readStringUntil('\n'); // read string until newline character

        if (command == "force resfresh")
        {
            getCurrentSong(&current_track, &token, &refresh_token);
        }
        else if (command == "next")
        {
            skipToNextSong(&token, &refresh_token);
        }
        else if (command == "previous")
        {
            skipToPreviousSong(&token, &refresh_token);
        }
        else if (command == "playpause")
        {
            togglePlayPause(&token, &refresh_token);
        }
        else if (command.startsWith("volume="))
        {
            Serial.println("Setting volume");
            Serial.println(command);
            Serial.println(command.substring(7));
            int volume = command.substring(7).toInt();
            setVolumePercentage(volume, &token, &refresh_token);
        }
    }

    if (IO_PINS){
        int volume = map_volume(analogRead(VOLUME_PIN));
        // volume +- 5% to avoid too many requests

        if (volume != current_track.volume_percent && (volume < previous_volume - 5 || volume > previous_volume + 5)){
            Serial.println("Setting volume");
            Serial.println(volume);
            setVolumePercentage(volume, &token, &refresh_token);
            delay(500);
            previous_volume = volume;
        }
    }

    progress_ms = millis() - start_time;
}
