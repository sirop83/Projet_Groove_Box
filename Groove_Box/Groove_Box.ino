#include "Config.h"

// --- INITIALISATION DES VARIABLES (Déclarées dans Config.h) ---
MachineState currentState = STATE_BOOT;
LiveSubState liveMode = SELECT_TRACK;

int currentKit = 1;
float currentBPM = 150.0;
unsigned long loopLengthMs = (60000.0 / 150.0) * 16;
bool trackActive[4] = {false, false, false, false};
float trackVolumes[4] = {1.0, 1.0, 1.0, 1.0};
int selectedTrackIdx = 0;
unsigned long nextLoopTime = 0;

unsigned long dspPopUpTimer = 0;
bool showDspPopUp = false;
int dspValue = 0;

// Écran I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Encodeur et Bouton
Encoder myEnc(PIN_ENC_A, PIN_ENC_B);
Bounce encBtn = Bounce();

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  setupControls(); // Logique dans Controls.ino
  // setupAudio(); // À décommenter quand Mathys remettra le code audio
}

void loop() {
  updateControls(); // Lecture de l'encodeur et des boutons

  u8g2.clearBuffer(); // On efface l'écran à chaque frame

  switch (currentState) {
    case STATE_BOOT:
      drawBootScreen();
      break;

    case STATE_MENU:
      drawMenuScreen();
      break;

    case STATE_LIVE:
      // runAudioEngine(); // Le séquenceur audio
      drawLiveScreen();
      break;
  }

  u8g2.sendBuffer(); // On envoie l'image à l'écran
}