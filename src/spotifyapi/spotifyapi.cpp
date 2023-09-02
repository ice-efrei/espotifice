#include "spotifyapi.h"
#include "../tftdisplay/tftdisplay.h"

int spotify_api_setup(
    TFT_eSPI *tft, 
    String *token,
    String *refresh_token,
    CurrentTrackDetails *current_track,
    SongDetails *song,
    int *progress_ms,
    int *start_time,
    int *last_refresh,
    int *last_state_ms){

    // Initialize variables
    *token = "";
    *refresh_token = "";
    *song = {
        0,
        0,
        "no album",
        "no image",
        "no artist",
        "no song"
    };
    *current_track = {
        0,
        0,
        false,
        song
    };
    
    // Get user access token
    while ((*token) == "")
    {
        // Wait until a user connect himself to the station
        ask_login_tft_display(tft);
        Serial.println("No token");
        delay(1000);
    }

    // Initialize the displayed song
    if (!getCurrentSong(current_track, token, refresh_token)) // if return 0
    {
        Serial.println("Current song: " + current_track->item->name);
        Serial.println("Current artist: " + current_track->item->artist_name);
        Serial.println("Current album image: " + current_track->item->album_image);
        
        // Initialize progress and refresh variables
        *progress_ms = current_track->progress_ms; 
        *start_time = millis() - *progress_ms;
    }
    else Serial.println("Error getting current song");

    *last_refresh = millis();
    *last_state_ms = millis();
    
    clear_tft_display(tft);
    
    return 0;
}

int clientCodeValidator(String code_param, String *token, String *refresh_token){
    /**
     * @brief Validate a code from the Spotify API by requesting an access token
    */
    Serial.println("Code param: " + code_param);
    return getUserAccessToken(code_param, token, refresh_token);
}

int getUserAccessToken(String serverCode, String *token, String *refresh_token) {
    HTTPClient http;
    String url = SPOTIFY_AUTHENTICATION_TOKEN_URL;
    String http_request_body = "grant_type=authorization_code&code=" + serverCode + "&redirect_uri=" + AUTHENTICATION_URL;

    Serial.println("http_request_body: " + http_request_body);

    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Basic " + base64::encode(CLIENT_ID + ":" + CLIENT_SECRET));
    http.addHeader("Content-Length", String(http_request_body.length()));
    http.addHeader("User-Agent", "ESP32");
    int httpCode = http.POST(http_request_body);

    if (httpCode != 200) {
        Serial.println("Error getting user access token (error 0 if 1)"); // https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/src/HTTPClient.h#L46
        return httpCode == 0 ? 1 : httpCode;
    }

    String payload = http.getString();
    Serial.println(payload);

    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);

    *token = root["access_token"].as<String>();
    *refresh_token = root["refresh_token"].as<String>();
    return 0;
}

int refreshToken(String *token, String *refresh_token){
    /**
     * @brief Refresh the access token
    */
    HTTPClient http;
    String url = SPOTIFY_AUTHENTICATION_TOKEN_URL;
    String http_request_body = "grant_type=refresh_token&refresh_token=" + *refresh_token;

    Serial.println("http_request_body: " + http_request_body);

    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Basic " + base64::encode(CLIENT_ID + ":" + CLIENT_SECRET));
    http.addHeader("Content-Length", String(http_request_body.length()));
    http.addHeader("User-Agent", "ESP32");
    int httpCode = http.POST(http_request_body);

    if (httpCode != 200) {
        Serial.println("Error refreshing user access token (error 0 if 1)");
        return httpCode == 0 ? 1 : httpCode;
    }

    String payload = http.getString();
    Serial.println(payload);

    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);

    *token = root["access_token"].as<String>();
    return 0;
}

int getCurrentSong(CurrentTrackDetails *current_track, String *token, String *refresh_token){
    /**
     * @brief Get the current song playing on the user's Spotify account
    */
    HTTPClient http;
    String url = "https://api.spotify.com/v1/me/player/currently-playing";
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + *(token));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("User-Agent", "ESP32");
    int httpCode = http.GET();

    if (httpCode == 401) {
        Serial.println("Refreshing token");
        if (refreshToken(token, refresh_token) != 0){
            Serial.println("Error refreshing token");
            return 1;
        }
        Serial.println("Token refreshed");
        return getCurrentSong(current_track, token, refresh_token);
    }
    else if (httpCode != 200) {
        Serial.println("Error getting current song:");
        Serial.println(httpCode);
        return httpCode == 0 ? 1 : httpCode;
    }

    String payload = http.getString();
    // Serial.println(payload);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);

    current_track->progress_ms = root["progress_ms"].as<int>();
    current_track->volume_percent = root["device"]["volume_percent"].as<int>();
    current_track->is_playing = root["is_playing"].as<bool>();
    current_track->item->duration_ms = root["item"]["duration_ms"].as<int>();
    current_track->item->popularity = root["item"]["popularity"].as<int>();
    current_track->item->album_name = root["item"]["album"]["name"].as<String>();
    current_track->item->name = root["item"]["name"].as<String>();
    current_track->item->id = root["item"]["id"].as<String>();
    String album_image = root["item"]["album"]["images"][0]["url"].as<String>();
    current_track->item->album_image = IMAGE_RESIZE_API + "?width=" + IMAGE_SIZE + "&height=" + IMAGE_SIZE + "&url=" + album_image;

    String artists = "";
    for (int i = 0; i < root["item"]["artists"].size(); i++)
    {
        artists += root["item"]["artists"][i]["name"].as<String>();
        if (i != root["item"]["artists"].size() - 1)
        {
            artists += ", ";
        }
    }
    current_track->item->artist_name = artists;

    return 0;
}

int setVolumePercentage(int volume, String *token, String *refresh_token){
    /**
     * @brief Set the volume of the user's Spotify account
    */
    Serial.println("Set volume to " + String(volume));
    HTTPClient http;
    String url = "https://api.spotify.com/v1/me/player/volume?volume_percent=" + String(volume);
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + *(token));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Length", "0");
    http.addHeader("User-Agent", "ESP32");
    int httpCode = http.PUT("");

    if (httpCode == 401) {
        Serial.println("Refreshing token");
        if (refreshToken(token, refresh_token) != 0){
            Serial.println("Error refreshing token");
            return 1;
        }
        Serial.println("Token refreshed");
        return setVolumePercentage(volume, token, refresh_token);
    }
    else if (httpCode != 204) {
        Serial.println("Error setting volume:");
        Serial.println(httpCode);
        return httpCode == 0 ? 1 : httpCode;
    }

    return 0;
}

int togglePlayPause(String *token, String *refresh_token){
    /**
     * @brief Toggle the play/pause of the user's Spotify account
    */
    Serial.println("Toggle play/pause");
    HTTPClient http;
    String url = "https://api.spotify.com/v1/me/player/play";
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + *(token));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Length", "0");
    http.addHeader("User-Agent", "ESP32");
    int httpCode = http.PUT("");

    if (httpCode == 401) {
        Serial.println("Refreshing token");
        if (refreshToken(token, refresh_token) != 0){
            Serial.println("Error refreshing token");
            return 1;
        }
        Serial.println("Token refreshed");
        return togglePlayPause(token, refresh_token);
    }
    else if (httpCode != 204) {
        Serial.println("Error toggling play/pause:");
        Serial.println(httpCode);
        return httpCode == 0 ? 1 : httpCode;
    }

    return 0;
}

int skipToNextSong(String *token, String *refresh_token){
    /**
     * @brief Skip to the next song of the user's Spotify account
    */
    Serial.println("Skip to next song");
    HTTPClient http;
    String url = "https://api.spotify.com/v1/me/player/next";
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + *(token));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Length", "0");
    http.addHeader("User-Agent", "ESP32");
    int httpCode = http.POST("");

    if (httpCode == 401) {
        Serial.println("Refreshing token");
        if (refreshToken(token, refresh_token) != 0){
            Serial.println("Error refreshing token");
            return 1;
        }
        Serial.println("Token refreshed");
        return skipToNextSong(token, refresh_token);
    }
    else if (httpCode != 204) {
        Serial.println("Error skipping to next song:");
        Serial.println(httpCode);
        return httpCode == 0 ? 1 : httpCode;
    }

    return 0;
}

int skipToPreviousSong(String *token, String *refresh_token){
    /**
     * @brief Skip to the previous song of the user's Spotify account
    */
    Serial.println("Skip to previous song");
    HTTPClient http;
    String url = "https://api.spotify.com/v1/me/player/previous";
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + *(token));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Length", "0");
    http.addHeader("User-Agent", "ESP32");
    int httpCode = http.POST("");

    if (httpCode == 401) {
        Serial.println("Refreshing token");
        if (refreshToken(token, refresh_token) != 0){
            Serial.println("Error refreshing token");
            return 1;
        }
        Serial.println("Token refreshed");
        return skipToPreviousSong(token, refresh_token);
    }
    else if (httpCode != 204) {
        Serial.println("Error skipping to previous song:");
        Serial.println(httpCode);
        return httpCode == 0 ? 1 : httpCode;
    }

    return 0;
}
