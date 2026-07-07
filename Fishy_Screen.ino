#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h> 
#include <Background.c>

//MQTT information
const char* MQTT_HOST = "23c7c9e727f2450999e63ac8d5f5eda0.s1.eu.hivemq.cloud";
const int   MQTT_PORT = 8883;
const char* MQTT_USER = "FishFish";
const char* MQTT_PASS = "BlubBlubBlub00";

WiFiClientSecure espClient;
PubSubClient client(espClient);

//screen constants
#define TFT_SCLK 8   //D8, GPIO8
#define TFT_COPI 10  //D10, GPIO10
#define TFT_RST 5    //D3, GPIO5
#define TFT_DC 4     //D2, GPIO4
#define TFT_CS 3     //D1, GPIO3
#define TFT_BL 2     //D0, GPIO2

//motion sensor
#define MOTION 20  //D7, GPIO20

//home, away
#define DND_BUTTON 7     //D5, GPIO7
#define PRESENCE_BUTTON 6  //D4, GPIO6

#define DND -1
#define HOME 1
#define AWAY 0

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_COPI, TFT_SCLK, TFT_RST, -1);

#define TEXT_X 60
#define TEXT_Y 110
#define TEXT_W 160   // adjust to fit your longest string ("DND ON" etc.)
#define TEXT_H 20

void showHome(){ // only show coral reef fish
  /*tft.fillRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, 0x0000); // only clear the text area
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(TEXT_X, TEXT_Y);
  tft.print("AT HOME");*/
  
  tft.drawRGBBitmap(0, 0, (const uint16_t*)Background_map, 240, 240);
}

void showDND(){ //deep sea fish
  tft.fillRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, 0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(TEXT_X, TEXT_Y);
  tft.print("DND ON");
}

void showAway(){ //fisherman
  tft.fillRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, 0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(TEXT_X, TEXT_Y);
  tft.print("AWAY");
}

void setup() {
  client.setServer(MQTT_HOST, MQTT_PORT);
  pinMode(MOTION, INPUT_PULLDOWN);
  pinMode(TFT_BL, OUTPUT);

  pinMode(DND_BUTTON, INPUT_PULLDOWN);
  pinMode(PRESENCE_BUTTON, INPUT_PULLDOWN);

  digitalWrite(TFT_BL, HIGH);

  tft.begin();
  tft.setSPISpeed(80000000);
  showHome();
  Serial.begin(9600);
}

bool lastMotionState = false;
bool firstRun = true;

bool presence_state = true;
bool dnd_state = false;

bool lastPresence = false;
bool lastDnd = false;

void loop() {
  bool presenceRead = digitalRead(PRESENCE_BUTTON);
  bool dndRead = digitalRead(DND_BUTTON);

  bool presencePressed = (presenceRead && !lastPresence);
  bool dndPressed = (dndRead && !lastDnd);

//we only want to run this code when the button goes from low to high
  if (presencePressed || dndPressed) { 
    if (presencePressed) {
      presence_state = !presence_state;
      Serial.println("Grey button pressed");
    }
    if (dndPressed && presence_state) {
      dnd_state = !dnd_state;
      Serial.println("Red button pressed");
    }
    
    if (presence_state && !dnd_state) {
      showHome();
    }
    
    else if (presence_state && dnd_state) {
      showDND();
    }
    
    else { //Away + DND or Away + !DND
      showAway();
    }
  }
  lastPresence = presenceRead;
  lastDnd = dndRead;
}



//motion stuffs
/*
bool motionState = digitalRead(MOTION);
  
  if (motionState != lastMotionState || firstRun) {
    if (motionState) {
      tft.setTextColor(GC9A01A_WHITE);
      tft.setTextSize(2);
      tft.setCursor(60, 110);
      tft.print("BUBBATRON");

      delay(2000);
    } else {
      tft.setTextColor(GC9A01A_BLACK);
      tft.setTextSize(2);
      tft.setCursor(60, 110);
      tft.print("BUBBATRON");
    }

    lastMotionState = motionState;
    firstRun = false;
} 

  delay(50);
*/
