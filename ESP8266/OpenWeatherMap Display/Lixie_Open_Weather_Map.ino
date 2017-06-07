/* -------------------------------------------------
   Open Weather Map Display
   using ESP8266 and Lixie Displays!

   by Connor Nishijima - 12/22/2016
   -------------------------------------------------

   To use your Lixie Displays / ESP8266 as a live
   weather readout for Open Weather Map, you'll need
   a few things:

   - WIFI_SSID
   - WIFI_PASSWORD
   - OWM_API_KEY
   - OWM_CITY_ID
   - OWM_FIELD
   - OWM_UNITS

   OWM_API_KEY can be generated by signing up for an
   API key with OWM at:
    https://openweathermap.org/appid

   OWM_CITY_ID can be found by searching for your
   local weather on openweathermap.org, and looking
   at the URL:

     Los Angeles:
     https://openweathermap.org/city/5368361
                This is your City ID ^

   OWM_FIELD is the type of weather reading you'd like
   to show. Options are:
     "temp",
     "pressure",
     "humidity",
     "temp_min",
     "temp_max"

   OWM_UNITS is how you'd like the information shown.
   Options are:
    "imperial",
    "metric",
    "kelvin"

   Another feature of this code is pulling your
   current weather state (cloudy, sunny, thunder,
   etc.) and mapping it to the color your your Lixies!

    (Very Pale) Blue   = Thunderstorm
    (Pale) Blue        = Drizzle
    Cyan               = Rain
    White              = Snow
    Gray               = Atmospheric (fog, pollution)
    Yellow             = Clear
    (Pale) Yellow      = Clouds / Calm
    Red                = Extreme (tornado, lightning)
   
   -------------------------------------------------
*/

#include "Lixie.h" // Include Lixie Library
#define DATA_PIN   5
#define NUM_LIXIES 4
Lixie lix(DATA_PIN, NUM_LIXIES);

#include <ESP8266WiFi.h>        // ESP8266 WIFI Lib
#include <ESP8266WiFiMulti.h>   // WifiMulti Lib for connection handling
#include <ESP8266HTTPClient.h>  // HTTPClient for web requests
#include <ArduinoJson.h>        // JSON Parser
ESP8266WiFiMulti WiFiMulti;

char* WIFI_SSID    = "";
char* WIFI_PASS    = "";

String OWM_API_KEY = "";           // Open Weather Map API Key
String OWM_CITY_ID = "";           // Open Weather Map CityID
String OWM_FIELD   = "temp";       // can be "temp","pressure","humidity","temp_min", or "temp_max"
String OWM_UNITS   = "imperial";   // can be "imperial", "metric", or "kelvin"

byte state_colors[9][3] = {
  {127,127,255}, // 0 Thunderstorm
  {64,64,255},   // 1 Drizzle
  {0,127,255},   // 2 Rain
  {255,255,255}, // 3 Snow
  {127,127,127}, // 4 Atmospheric
  {255,255,0},   // 5 Clear
  {255,255,64},  // 6 Clouds 
  {255,0,0},     // 7 Extreme
  {255,255,64}   // 8 Calm  
};

void setup() {
  Serial.begin(115200);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS); // Your WIFI credentials
  lix.begin(); // Initialize LEDs

  // This sets all lights to yellow while we're connecting to WIFI
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    lix.color(255, 255, 0);
    lix.write(8888);
    delay(100);
  }

  // Green on connection success
  lix.color(0, 255, 0);
  lix.write(8888);
  delay(500);

  // Reset colors to default
  lix.color(255, 255, 255);
  lix.clear();
}

void loop() {
  checkOWM();
  delay(30000);
}

// FUNCTIONS ----------------------------------------

// Check latest data.sparkfun.com log
void checkOWM() {
  StaticJsonBuffer<1200> jsonBuffer;      // JSON Parser setup
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://api.openweathermap.org/data/2.5/weather?id="+OWM_CITY_ID+"&appid="+OWM_API_KEY+"&units="+OWM_UNITS);
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
        JsonObject& owm_data = jsonBuffer.parseObject(payload);
        if (!owm_data.success()) {
          Serial.println("Parsing failed");
          return;
        }
        int field = owm_data["main"][OWM_FIELD];
        int code = owm_data["weather"][0]["id"];
        int weather_state = codeToState(code);
        lix.color(
          state_colors[weather_state][0],
          state_colors[weather_state][1],
          state_colors[weather_state][2]
        );
        lix.write(field);
      }
    }
    http.end();
  }
}

byte codeToState(uint16_t code){
  byte state = 0;
  if(code >= 200 && code < 300){
    state = 0;
  }
  else if(code >= 300 && code < 400){
    state = 1;
  }
  else if(code >= 500 && code < 600){
    state = 2;
  }
  else if(code >= 600 && code < 700){
    state = 3;
  }
  else if(code >= 700 && code < 800){
    state = 4;
  }
  else if(code == 800){
    state = 5;  
  }
  else if(code > 800 && code < 900){
    state = 6;
  }
  else if(code >= 900 && code < 907){
    state = 7;
  }
  else if(code >= 907 && code < 956){
    state = 8;
  }
  else if(code >= 956){
    state = 7;
  }
  return state;
}
