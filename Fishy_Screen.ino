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
#include "away_musicnotes1.c"

//MQTT information
const char* MQTT_HOST = "23c7c9e727f2450999e63ac8d5f5eda0.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
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

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_COPI, TFT_SCLK, TFT_RST, -1);

//motion sensor
#define MOTION 20  //D7, GPIO20

//home, away
#define DND_BUTTON 7       //D5, GPIO7
#define PRESENCE_BUTTON 6  //D4, GPIO6

//old definitions for printing out status
/*#define TEXT_X 60
#define TEXT_Y 110
#define TEXT_W 160 
#define TEXT_H 20*/

//fish animation (home + DND)
#define MAGENTA 0xF81F  //make this magenta color act as a transparent block

enum BiomeType {
  CORAL_REEF,
  DEEP_SEA,
  AWAY_BIOME
};

BiomeType biome = CORAL_REEF;

//Sprites for coral reef
const uint16_t* Pink_map[5] = {
  (const uint16_t*)Pink_1_map[0],
  (const uint16_t*)Pink_1_map[1],
  (const uint16_t*)Pink_1_map[2],
  (const uint16_t*)Pink_1_map[3],
  (const uint16_t*)Pink_1_map[4]
};

const uint16_t* Jellyfish_map[5] = {
  (const uint16_t*)jellyfish1_map[0],
  (const uint16_t*)jellyfish1_map[1],
  (const uint16_t*)jellyfish1_map[2],
  (const uint16_t*)jellyfish1_map[3],
  (const uint16_t*)jellyfish1_map[4]
};

const uint16_t* Nemo_map[5] = {
  (const uint16_t*)nemo_1_map[0],
  (const uint16_t*)nemo_1_map[1],
  (const uint16_t*)nemo_1_map[2],
  (const uint16_t*)nemo_1_map[3],
  (const uint16_t*)nemo_1_map[4]
};

const uint16_t* angler_map[5] = {
  (const uint16_t*)angler1_map[0],
  (const uint16_t*)angler1_map[1],
  (const uint16_t*)angler1_map[2],
  (const uint16_t*)angler1_map[3],
  (const uint16_t*)angler1_map[4]
};

const uint16_t* squid_map[5] = {
  (const uint16_t*)squid1_map[0],
  (const uint16_t*)squid1_map[1],
  (const uint16_t*)squid1_map[2],
  (const uint16_t*)squid1_map[3],
  (const uint16_t*)squid1_map[4]
};

const uint16_t* Grey_map[5] = {
  (const uint16_t*)Grey_1_map[0],
  (const uint16_t*)Grey_1_map[1],
  (const uint16_t*)Grey_1_map[2],
  (const uint16_t*)Grey_1_map[3],
  (const uint16_t*)Grey_1_map[4]
};

//Sprites for coral reef fish
const uint16_t** coralReefFishU16[3] = {
  Jellyfish_map,
  Pink_map,
  Nemo_map
};

//Sprites for deep sea fish
const uint16_t** deepSeaFishU16[3] = {
  angler_map,
  squid_map,
  Grey_map
};

//Sprites for Away
const uint16_t* musicNotes_map[2] = {
  (const uint16_t*)away_musicnotes1_map[0],
  (const uint16_t*)away_musicnotes1_map[1]
};

//biome type pointer
const uint16_t* getBiomeMap(BiomeType biome) {
  switch (biome) {
    case CORAL_REEF: return (const uint16_t*)Coral_Reef_map;
    case DEEP_SEA: return (const uint16_t*)Deep_Sea_map;
    case AWAY_BIOME: return (const uint16_t*)Away1_map;
  }
  return (const uint16_t*)Coral_Reef_map;
}

//Chooses a random fish to spawn depending on the 'biome'
const uint16_t** chooseRandomFish(BiomeType biome) {
  int randomIndex = random(0, 3);
  if (biome == CORAL_REEF) {
    return coralReefFishU16[randomIndex];
  }
  if (biome == DEEP_SEA) {
    return deepSeaFishU16[randomIndex];
  }
}

//creates a background patch corresponding to 240x240 map
void patchUpdate(const uint16_t* fullMap, int x0, int y0, int w, int h) {
  uint16_t patch[w * h];
  for (int row = 0; row < h; row++) {
    const uint16_t* srcRow = fullMap + (y0 + row) * 240 + x0;
    memcpy(&patch[row * w], srcRow, w * sizeof(uint16_t));
  }
  tft.drawRGBBitmap(x0, y0, patch, w, h);
}

//draws the sprite (fish or music), ignoring magenta pixels (0x1FF8)
void drawSprite(const uint16_t* spriteFrame, int x0, int y0, int w, int h) {
  uint16_t rowBuf[w];
  for (int row = 0; row < h; row++) {
    int col = 0;
    while (col < w) {
      if (spriteFrame[row * w + col] == MAGENTA) {  //ignore magenta blocks
        col++;
        continue;
      }
      int runstart = col;
      int runLen = 0;
      while (col < w && spriteFrame[row * w + col] != MAGENTA) {
        rowBuf[runLen] = spriteFrame[row * w + col];
        runLen++;
        col++;
      }
      tft.drawRGBBitmap(x0 + runstart, y0 + row, rowBuf, runLen, 1);
    }
  }
}

//timing variables
int currFrame_fish = 0;
unsigned long lastFrameTime_fish = 0;
const unsigned long frameInterval_fish = 100;

int currentY = random(20, 220);
int currentX = -20;
int speedX = 3;
const uint16_t** currentFish = chooseRandomFish(biome);

/*Flips through the swimming frames of the fish. Draws fish, wipes, updates next spawn location, draws next frame
Chooses a random y-spawn for the fish*/
void animateSwim() {
  unsigned long currTime_fish = millis();
  if (currTime_fish - lastFrameTime_fish > frameInterval_fish) {
    lastFrameTime_fish = currTime_fish;
    patchUpdate(getBiomeMap(biome), currentX, currentY, 20, 20);
    currFrame_fish = (currFrame_fish + 1) % 5;
    currentX += speedX;
    if (currentX > 240) {
      currentX = -20;
      currentY = random(20, 220);
      speedX = 3;
      currentFish = chooseRandomFish(biome);
      currFrame_fish = 0;
    }
    drawSprite(currentFish[currFrame_fish], currentX, currentY, 20, 20);
  }
}
// Background show functions
void showHome() {  
  tft.drawRGBBitmap(0, 0, (const uint16_t*)Coral_Reef_map, 240, 240);
}
void showDND() {  //deep sea fish
  tft.drawRGBBitmap(0, 0, (const uint16_t*)Deep_Sea_map, 240, 240);
}
void showAway() {  //fisherman
  tft.drawRGBBitmap(0, 0, (const uint16_t*)Away1_map, 240, 240);
}

//timing
int currFrame_away = 0;
unsigned long lastFrameTime_away = 0;
const unsigned long frameInterval_away = 400;

//Away patch coordinates
const int PATCH_X = 118;
const int PATCH_Y = 84;
const int PATCH_W = 18;
const int PATCH_H = 25;

//animates the music notes. Draws a frame, patches, draws another frame on top
void animateMusic() {
  int currTime_away = millis();
  if (currTime_away - lastFrameTime_away > frameInterval_away) {
    lastFrameTime_away = currTime_away;
    patchUpdate(getBiomeMap(biome), PATCH_X, PATCH_Y, PATCH_W, PATCH_H);
    currFrame_away = (1 + currFrame_away) % 2;
    drawSprite(musicNotes_map[currFrame_away], PATCH_X, PATCH_Y, PATCH_W, PATCH_H);
  }
}

//MQTT server setup for esp32c3 pairing
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

//motion detection variables
bool lastMotionState = false;
bool firstRun = true;

//variables for detecting button press
bool presence_state = true;
bool dnd_state = false;
bool lastPresence = false;
bool lastDnd = false;

//variables for defining statuses
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

    else {  //Away + DND or Away + !DND
      showAway();
      currentStatus = AWAY;
    }
  }

  if (currentStatus == HOME) {
    biome = CORAL_REEF;
    animateSwim();
  }

  if (currentStatus == DND) {
    biome = DEEP_SEA;
    animateSwim();
  }

  if (currentStatus == AWAY) {
    biome = AWAY_BIOME;
    animateMusic();
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
