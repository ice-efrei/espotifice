#ifndef TFTDISPLAY_H
#define TFTDISPLAY_H

#include <TFT_eSPI.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
#include "../../user_settings.h"
#include "../spotifyapi/spotifyapi.h"

void setup_tft_display(TFT_eSPI *tft, bool (*tft_output)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap));
void clear_tft_display(TFT_eSPI *tft);
void wifi_setup_tft_display(TFT_eSPI *tft, int trials);
void display_wifi_connected(TFT_eSPI *tft);
void ask_login_tft_display(TFT_eSPI *tft);
void display_current_song(TFT_eSPI *tft, CurrentTrackDetails *current_track, int state);
void display_error_tft_display(TFT_eSPI *tft, String error);
void display_album_image(TFT_eSPI *tft, uint8_t *thumbnail, int iDataSize, int pos_x, int pos_y, int w, int h);
int getFileToArray(String url, uint8_t **image_array, int *image_size);

// effects
void scrolling_text(TFT_eSPI *tft, String text, int pos_x, int pos_y, int state);

#endif