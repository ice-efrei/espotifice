# ESPotifICE

## Project Overview

ESPotifICE is an ICE EFREI project that focuses on creating a Spotify control base using an ESP32. The project aims to display the currently playing song on a Spotify account on a TFT display. This allows users to have a visual representation of the song that is currently being played on Spotify. The project also allows users to control the playback of the song using the ESP32. The ESP32 can be connected to a local WiFi network and can be accessed via a web browser to authenticate with a Spotify account. Once authenticated, the ESP32 will display the currently playing song information and album image on the TFT display. The ESP32 can also be controlled using the web browser to skip to the next song, skip to the previous song, toggle play/pause, and set the volume. The ESP32 can also be controlled using an analog potentiometer connected to the ESP32's GPIO pins (IO_PINS should be set to true in the code).

## Technologies Used

The project utilizes several technologies and components to achieve its functionality:

- **ESP32**: The ESP32 microcontroller serves as the core hardware platform for the project.
- **WiFi**: The ESP32 connects to a local WiFi network to access the internet and communicate with the Spotify API.
- **AsyncWebServer**: This library is used to create a local web server on the ESP32 to handle authentication and interactions with the Spotify API.
- **HTTPClient**: The HTTPClient library is used to make HTTP requests to the Spotify API for retrieving song information.
- **TFT_eSPI**: The TFT_eSPI library is used to interface with the TFT display and render the song information and album images.
- **ArduinoJson**: ArduinoJson is used for parsing JSON responses from the Spotify API.
- **base64**: The base64 library is used for handling base64 encoded data.
- **TJpg_Decoder**: This library is used for decoding and displaying JPEG images on the TFT display.
- **Arduino IDE**: The Arduino IDE is used as the development environment for programming the ESP32.

## How to Use the Project

1. Connect the ESP32 to a local WiFi network.
2. Access the ESP32's local web server via a web browser.
3. Follow the prompts to authenticate with your Spotify account and grant access to the ESP32.
4. Once authenticated, the ESP32 will display the currently playing song information and album image on the TFT display.
5. You can use the following commands to interact with the project:
   - "force refresh": Force the ESP32 to refresh the current song information from the Spotify API.
   - "next": Skip to the next song.
   - "previous": Skip to the previous song.
   - "playpause": Toggle play/pause of the current song.
   - "volume=X": Set the volume percentage to X (0-100).
6. You can also control the volume using an analog potentiometer connected to the ESP32's GPIO pins (IO_PINS should be set to true in the code).

## Wire Diagram
![Wire Diagram](https://raw.githubusercontent.com/ice-efrei/espotifice/main/ESPostifICE.png)

## References

- [ESPotifICE GitHub Repository](https://github.com/ice-efrei/espotifice)
- [Interfacing ESP8266 NodeMCU with ST7789 TFT Display](https://simple-circuit.com/esp8266-nodemcu-st7789-tft-ips-display/)
- [Web API | Spotify for Developers](https://developer.spotify.com/documentation/web-api)
- [ArduinoJson Documentation](https://arduinojson.org/)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)
- [TJpg_Decoder Library](https://github.com/Bodmer/TJpg_Decoder)
- [Other ESP32 Spotify Project](https://gitlab.com/makeitforless/spotify_controller)

## Note

Please refer to the [ice-efrei.fr](https://ice-efrei.fr) for any additional description.