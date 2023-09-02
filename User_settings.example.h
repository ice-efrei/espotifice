#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

// General settings

// WiFi credentials
#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"

// Spotify API credentials
const String CLIENT_ID = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const String CLIENT_SECRET = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

// Spotify API endpoints
const String AUTHENTICATION_URL = "https://espotifice.ice-efrei.fr";
const String SPOTIFY_AUTHENTICATION_URL = "https://accounts.spotify.com";
const String SPOTIFY_AUTHENTICATION_TOKEN_URL = SPOTIFY_AUTHENTICATION_URL + "/api/token";
const String SPOTIFY_API_URL = "https://api.spotify.com/v1";
const String SPOTIFY_API_CURRENT_TRACK_URL = SPOTIFY_API_URL + "/me/player";
const String IMAGE_RESIZE_API = "https://resize-image-api.vercel.app/api/resize"; // https://resize.sardo.work?imageUrl=https://i.scdn.co/image/ab67616d0000b2735675e83f707f1d7271e5cf8a&width=150&height=150

// Spotify API Variables
const String SPOTIFY_SCOPE = "user-read-playback-state user-modify-playback-state user-read-currently-playing";

// TFT display settings
#define MAX_IMAGE_WIDTH 150
#define IMAGE_SIZE 150
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define MAX_CHARACTERS_PER_LINE 19

#endif