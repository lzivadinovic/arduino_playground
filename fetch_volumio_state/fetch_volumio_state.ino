// wifi library
#include <ESP8266WiFi.h>

// http client
#include <ESP8266HTTPClient.h>

// JSON Parser
#include <ArduinoJson.h>
// Initialize DynamicJsonObject
DynamicJsonDocument doc(2048);


// Connections for ESP8266 hardware SPI are:
// Vcc       3v3     LED matrices seem to work at 3.3V
// GND       GND     GND
// DIN        D7     HSPID or HMOSI
// CS or LD   D8     HSPICS or HCS
// CLK        D5     CLK or HCLK
//

// For hardware type definition
#include <MD_MAX72xx.h>
// SPI interface and Parola library for animation
#include <MD_Parola.h>
#include <SPI.h>

// Define my hardware
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// How many 8x8 segments we use
#define MAX_DEVICES 4

// Pinout declaration
#define CLK_PIN   D5 // or SCK
#define DATA_PIN  D7 // or MOSI
#define CS_PIN    D8 // or SS

// SPI hardware interface; create display object
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Scrolling parameters
uint8_t frameDelay = 35;  // default frame delay value in ms
textEffect_t  scrollEffect = PA_SCROLL_LEFT;

// WiFi data
#ifndef WSSID
#define WSSID "WIFISSID"
#define WPSK  "WIFIPASS"
#endif

const char* ssid = WSSID;
const char* password = WPSK;


// http endpoint
const char* volumio_endpoint = "http://10.20.45.29/api/v1/getState";

// Buffer for messages
#define BUF_SIZE  512
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;

void setup() {

  Serial.begin(115200);
  Serial.println();

  // Intialize the display object:
  myDisplay.begin();
  // Clear the display:
  myDisplay.displayClear();
  // Set the intensity (brightness) of the display (0-15):
  myDisplay.setIntensity(5);
  // Start displayScroll animation
  myDisplay.displayScroll(curMessage, PA_LEFT, scrollEffect, frameDelay); //(curMessage, PA_LEFT, 50, 1000, PA_PRINT, PA_WIPE_CURSOR);
  // Put null byte at begining so parole wont display garbage
  curMessage[0] = newMessage[0] = '\0';

  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Start wifi connection to AP
  WiFi.begin(ssid, password);

  // Notify user that we are connecting to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Put ip in curMessage buffer
  sprintf(curMessage, "%d:%d:%d:%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);


}

void fetchHTTP(void){
   // Initialize client
    HTTPClient http;
    // Force http/1.0 because we bypass chunked transfer encoding using http.getStream()
    http.useHTTP10(true);
    // Specify endpoint
    http.begin(volumio_endpoint);
    // Send request
    int httpCode = http.GET();
    
    // Check if http 200
    if (httpCode == 200) {
      // Deserialize json so we can use it
      deserializeJson(doc, http.getStream());
      // Test print
      Serial.println(doc["service"].as<char*>());
      Serial.println(doc["title"].as<char*>());
      Serial.println(doc["artist"].as<char*>());
      // Send disconnect message to server (?)
      http.end();
    }
    // TODO concat strings
    // Copy from response to curMessage buffer
    strcpy(curMessage, doc["title"].as<char*>());
}


void loop() {
  // TODO
  // Display service every thrid animation as static text for ~3 sec
  // Some radios/services show separate title and artis, concatenate those strings
  
  if(myDisplay.displayAnimate()) {
    fetchHTTP();
    myDisplay.displayReset();
  }
}
