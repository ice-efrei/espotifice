#include "tftdisplay.h"

void setup_tft_display(TFT_eSPI *tft, bool (*tft_output)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)){
    tft->init();
    tft->setRotation(0);
    clear_tft_display(tft);

     // Initialize the decoder for thumbnails display
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);
}

void clear_tft_display(TFT_eSPI *tft){
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
}

void wifi_setup_tft_display(TFT_eSPI *tft, int trials){
    clear_tft_display(tft);
    String loading = "";
    for (int i = 0; i < trials%4; i++) loading += ".";
    tft->setCursor(40, 90);
    tft->println("Connecting to ");
    tft->setCursor(40, 125);
    tft->print(WIFI_SSID);
    tft->println(loading);
}

void display_wifi_connected(TFT_eSPI *tft){
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    // Show connected
    tft->setCursor(60, 110);
    tft->println("Connected!");
    delay(3000);
}

void ask_login_tft_display(TFT_eSPI *tft){
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    // tft->setTextSize(1);
    tft->setCursor(0, 0);
    tft->println("Please login to Spotify through our portal at:");
    tft->println("");
    tft->println("http://" + WiFi.localIP().toString() + "/");
    tft->println("");
    tft->println("Please ensure to be connected to the following WiFi network before accessing this website:");
    tft->println("");
    tft->println(WIFI_SSID);
}

void display_current_song(TFT_eSPI *tft, CurrentTrackDetails *current_track, int state){
    int pos_x = 0;
    int pos_y = SCREEN_HEIGHT - SCREEN_HEIGHT/6;

    String displayed_title = current_track->item->name + " - " + current_track->item->artist_name;

    tft->fillRect(pos_x, pos_y, SCREEN_WIDTH, SCREEN_HEIGHT/6, TFT_BLACK);
    scrolling_text(tft, displayed_title, pos_x, pos_y, state);

    // // test 
    // tft->setTextColor(TFT_WHITE);
    // tft->setTextSize(2);
    // tft->setCursor(pos_x, pos_y+20);
    // tft->print("0123456789012345678900123456789");
}

void display_error_tft_display(TFT_eSPI *tft, String error){
    tft->setTextColor(TFT_RED);
    tft->setTextSize(2);
    tft->setCursor(0, 2);
    tft->println("Error:");
    tft->println("");
    tft->println(error);
}

// void display_boot_screen()
// {
//   int16_t rc = png.openFLASH((uint8_t *)ice, sizeof(ice), pngDraw);
//   if (rc == PNG_SUCCESS) {
//     Serial.println("Successfully opened png file");
//     Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
//     tft.startWrite();
//     uint32_t dt = millis();
//     rc = png.decode(NULL, 0);
//     Serial.print(millis() - dt); Serial.println("ms");
//     tft.endWrite();
//     // png.close(); // not needed for memory->memory decode
//   }
//   delay(3000);
// }

// void pngDraw(PNGDRAW *pDraw) {
//   uint16_t lineBuffer[MAX_IMAGE_WIDTH];
//   png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
//   tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
// }

int getFileToArray(String url, uint8_t **image_array, int *image_size)
{
    Serial.println("Downloading from " + url);

    // Check WiFi connection
    if ((WiFi.status() != WL_CONNECTED)) {
        Serial.println("WiFi not connected");
        return 1;
    }

    Serial.print("[HTTP] begin...\n");

    // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // TODO with ArduinoBearSSL
    WiFiClientSecure *client = new WiFiClientSecure;
    client -> setInsecure();
    HTTPClient http;
    http.begin(*client, url);

    Serial.print("[HTTP] GET...\n");
    
    // free old array (not enough memory to keep both)
    free(*image_array);

    // Start connection and send HTTP header
    int httpCode = http.GET();

    if (httpCode != 200) {
        Serial.println("Error getting thumbnail :");
        Serial.println(httpCode);
        return 1;
    }

    // Get length of document (is -1 when Server sends no Content-Length header)
    int total = http.getSize();
    int len = total;

    Serial.println("Total size : " + String(total));

    // Create buffer for read
    uint8_t buff[128] = { 0 };

    // Create array for image
    uint8_t *image = (uint8_t *)malloc(total);

    // Get tcp stream
    WiFiClient * stream = http.getStreamPtr();

    Serial.println("Reading data from server");

    // Read all data from server
    int i = 0;
    while (http.connected() && (len > 0 || len == -1)) {
        // Get available data size
        size_t size = stream->available();

        if (size) {
            // Read up to 128 bytes
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

            // Write it to file
            memcpy(image + i, buff, c);
            i += c;

            // Calculate remaining bytes
            if (len > 0) {
                len -= c;
            }
        }

        // Display progress in percentage
        Serial.print("Progress : ");
        Serial.print((int)((float)(total - len) / total * 100));
        Serial.println("%");
    }
    http.end();

    *image_array = image;
    *image_size = total;
    return 0;
}

void display_album_image(TFT_eSPI *tft, uint8_t *thumbnail, int iDataSize, int pos_x, int pos_y, int w, int h)
{
    Serial.println("Displaying album image");
    uint32_t t = millis();

    // Draw the image, top left at 0,0
    TJpgDec.drawJpg(pos_x, pos_y, thumbnail, iDataSize);

    // How much time did rendering take (ESP8266 80MHz 262ms, 160MHz 149ms, ESP32 SPI 111ms, 8bit parallel 90ms
    t = millis() - t;
    Serial.print(t); Serial.println(" ms");
}

void scrolling_text(TFT_eSPI *tft, String text, int pos_x, int pos_y, int state){
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->setCursor(pos_x, pos_y);
    
    if (text.length() < MAX_CHARACTERS_PER_LINE){
        tft->print(text);
        return;
    }
    
    int scrolling_state = state % (text.length() + 3);
    String scrolling_text = text + "  " + text + "   ";
    String displayed_text = scrolling_text.substring(scrolling_state, scrolling_state + MAX_CHARACTERS_PER_LINE);

    // Serial.println(displayed_text);
    tft->print(displayed_text);
}

void loading_thumbnail(TFT_eSPI *tft, int pos_x, int pos_y){
    // 
}
