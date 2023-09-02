#ifndef SPOTIFYAPI_H
#define SPOTIFYAPI_H
#include <HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "../../user_settings.h"

typedef struct SongDetails_t
{
    int duration_ms;
    int popularity;
    String album_name;
    String album_image;
    String artist_name; // Concatenate all artists name in a string
    String name;
    String id;
} SongDetails;

typedef struct CurrentTrackDetails_t
{
    int progress_ms;
    int volume_percent;
    bool is_playing;
    SongDetails *item;
} CurrentTrackDetails;

int spotify_api_setup(
    TFT_eSPI *tft, 
    String *token,
    String *refresh_token,
    CurrentTrackDetails *current_track,
    SongDetails *song,
    int *progress_ms,
    int *start_time,
    int *last_refresh,
    int *last_state_ms);
int clientCodeValidator(String code_param, String *token, String *refresh_token);
int getUserAccessToken(String serverCode, String *token, String *refresh_token);
int getCurrentSong(CurrentTrackDetails *current_track, String *token, String *refresh_token);
int refreshToken(String *token, String *refresh_token);
int setVolumePercentage(int volume, String *token, String *refresh_token);
int togglePlayPause(String *token, String *refresh_token);
int skipToNextSong(String *token, String *refresh_token);
int skipToPreviousSong(String *token, String *refresh_token);

#endif