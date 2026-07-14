#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h> 

#include "Coral_Reef.c"
#include "Nemo_1.c"
#include "Pink_1.c"
#include "jellyfish1.c"

#include "Deep_Sea.c"
#include "angler1.c"
#include "squid1.c"
#include "Grey_1.c"

#include "Away1.c"

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

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_COPI, TFT_SCLK, TFT_RST, -1);

#define TEXT_X 60
#define TEXT_Y 110
#define TEXT_W 160   // adjust to fit your longest string ("DND ON" etc.)
#define TEXT_H 20

//patch update for away 
const uint16_t* animFrames[4] = {
  (const uint16_t*)Away1_map[0],
  (const uint16_t*)Away1_map[1],
  (const uint16_t*)Away1_map[2],
  (const uint16_t*)Away1_map[3]
};

//fish animation (home + DND)
typedef const uint8_t (*FishAnimation)[800];

enum BiomeType {
  CORAL_REEF,
  DEEP_SEA
};

FishAnimation coralReefFish[3] = {
  jellyfish1_map,
  Pink_1_map, 
  nemo_1_map
};

FishAnimation deepSeaFish[3] = {
  angler1_map,
  squid1_map, 
  Grey_1_map
};


FishAnimation chooseRandomFish(BiomeType currentBiome){
  //Chooses a random fish depending on the 'biome'
  int randomIndex = random(0, 3);

  if (currentBiome == CORAL_REEF){
    return coralReefFish[randomIndex];
  }

  if(currentBiome == DEEP_SEA){
    return deepSeaFish[randomIndex];
  }
}

void animateSwim(FishAnimation fishToDraw){
  int screenWidth = 240; 
  int screenHeight = 240;
  int fishWidth = 20;  
  int fishHeight = 20;

  int currentY = random(20, screenHeight - fishHeight);

  int currentX = 0;
  int distanceX = 10; //distance between each frame
  int frameindex = 0;

  while (currentX < screenWidth){
    /* Draw fish, wipe, update next spawn location, draw next frame*/

  }
}

void patchUpdate(const uint16_t* fullMap, int x0, int y0, int w, int h, int img_width){
  //creates a patch on the away status to update the music notes
  uint16_t patch[w*h];
  for (int row = 0; row < h; row++){
    const uint16_t* srcRow = fullMap + (y0 + row) * img_width + x0;
    memcpy(&patch[row * w], srcRow, w*sizeof(uint16_t));
  }
  tft.drawRGBBitmap(x0, y0, patch, w, h);
}

void showHome(){ // only show coral reef fish
  tft.drawRGBBitmap(0, 0, (const uint16_t*)Coral_Reef_map, 240, 240);
}
void showDND(){ //deep sea fish
  tft.drawRGBBitmap(0, 0, (const uint16_t*)Deep_Sea_map, 240, 240);
}

void showAway(){ //fisherman
  tft.drawRGBBitmap(0, 0, (const uint16_t*)Away1_map, 240, 240);

}

//timing
int currFrame = 0;
unsigned long lastFrameTime = 0;
const unsigned long frameInterval = 400;

//Away patch coordinates
const int PATCH_X = 118;
const int PATCH_Y = 84;
const int PATCH_W = 20;
const int PATCH_H = 28;

int animateMusic(int currFrame){
//animates the music notes by going to the next frame
  int currTime = millis();
  if (currTime - lastFrameTime > frameInterval){
    lastFrameTime = currTime;
    currFrame = (1+currFrame)%4;
    patchUpdate(animFrames[currFrame], PATCH_X, PATCH_Y, PATCH_W, PATCH_H, 240);
  }
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

#define HOME 0
#define AWAY 1
#define DND 2

int currentStatus = HOME;

void loop() {
  bool presenceRead = digitalRead(PRESENCE_BUTTON);
  bool dndRead = digitalRead(DND_BUTTON);

  bool presencePressed = (presenceRead && !lastPresence);
  bool dndPressed = (dndRead && !lastDnd);

//we only want to run this code when the button goes from low to high
  if (presencePressed || dndPressed) { 
    if (presencePressed) {
      presence_state = !presence_state;
      Serial.println("Home/Away button pressed");
    }
    if (dndPressed && presence_state) {
      dnd_state = !dnd_state;
      Serial.println("DND button pressed");
    }
    
    if (presence_state && !dnd_state) {
      showHome();
      currentStatus = HOME;
    }
    
    else if (presence_state && dnd_state) {
      showDND();
      currentStatus = DND;
    }
    
    else { //Away + DND or Away + !DND
      showAway();
      currentStatus = AWAY;
    }
  }

  if (currentStatus == AWAY) {
    animateMusic(currFrame);
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
