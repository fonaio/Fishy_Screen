#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

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
#define STATUS_BUTTON 6  //D4, GPIO6

#define DND -1
#define HOME 1
#define AWAY 0

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_COPI, TFT_SCLK, TFT_RST, -1);

//If we are just home, show coral reef fish
void atHome(){
  
}

//If we are home + DND, show jellyfish
void DND(){


} 

//If we are away, show fisherman. Clicking the DND button when you are away does not do anything 
void away(){
  
}

void setup() {
  pinMode(MOTION, INPUT_PULLDOWN);
  pinMode(TFT_BL, OUTPUT);

  digitalWrite(TFT_BL, HIGH);

  tft.begin();
  tft.fillScreen(GC9A01A_BLACK);
  Serial.begin(9600);
}

bool lastMotionState = false;
bool firstRun = true;

int status = HOME

void loop() {
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
}
