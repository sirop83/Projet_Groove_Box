#include "Config.h"

// --- INITIALISATION DES VARIABLES ---
MachineState currentState = STATE_BOOT;
LiveSubState liveMode = SELECT_TRACK;

int currentKit = 1;
float currentBPM = 150.0;
unsigned long loopLengthMs = (60000.0 / 150.0) * 16;
bool trackActive[4] = {false, false, false, false};
float trackVolumes[4] = {0.5, 0.5, 0.5, 0.5};
int selectedTrackIdx = 0;
unsigned long nextLoopTime = 0;

unsigned long dspPopUpTimer = 0;
bool showDspPopUp = false;
bool isRunning = false;
int dspValue = 0;

unsigned long bootTimer = 0; // LE NOUVEAU CHRONOMÈTRE

// Écran I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Encodeur et Bouton
Encoder myEnc(PIN_ENC_A, PIN_ENC_B);
Bounce encBtn = Bounce();

void setup() {
  Serial.begin(115200);
  
  delay(500); // Laisse une demi-seconde à l'écran pour avoir du courant
  u8g2.begin();

  setupControls(); 
  setupAudio(); 
  
  bootTimer = millis(); // On déclenche le chrono ICI !
}

void loop() {
  updateControls(); 

  // PHASE 1 : L'écran de démarrage pendant 3 secondes
  if (currentState == STATE_BOOT) {
    drawBootScreen();
    
    if (millis() - bootTimer > 1500) { // Si 3 secondes sont passées
      currentState = STATE_MENU;
    }
  } 
  // PHASE 2 : L'interface normale
  else {
    u8g2.clearBuffer(); 

    if (currentState == STATE_MENU) {
      drawMenuScreen();
    } else if (currentState == STATE_LIVE) {
      runAudioEngine();
      drawLiveScreen();
    }

    u8g2.sendBuffer(); 
  }
}