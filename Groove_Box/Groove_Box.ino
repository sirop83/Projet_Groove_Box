#include "Config.h"

// --- INITIALISATION DES VARIABLES ---
MachineState currentState = STATE_BOOT;
LiveSubState liveMode = SELECT_TRACK;

int currentKit = 1;
float currentBPM = 150.0;
unsigned long loopLengthMs = (60000.0 / 150.0) * 16;
bool trackActive[4] = {false, false, false, false};
float trackVolumes[4] = {0.5, 0.5, 0.5, 0.5};
float trackFilters[4] = {15000.0, 15000.0, 15000.0, 15000.0};
int trackSound[4] = {0, 1, 2, 3};
int selectedTrackIdx = 0;
unsigned long nextLoopTime = 0;
int mainMenuSelection = 0;
int longPressProgress = 0;

int chosenRecordBtn = 0;
unsigned long recordTimer = 0;
bool isUsingMicPack = false;
int activeMicPackIdx = 0;

bool packExists[MAX_MIC_TRACKS] = {false, false, false, false};
int micMenuSelection = 0;
int selectedMicPackIdx = 0;
int micPackMenuSelection = 0;
int micDeleteConfirmSelection = 0;

unsigned long dspPopUpTimer = 0;
bool showDspPopUp = false;
bool isRunning = false;
int dspValue = 0;

unsigned long bootTimer = 0;

// --- MATÉRIEL ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Encoder myEnc(PIN_ENC_A, PIN_ENC_B);
Bounce encBtn = Bounce();

// --- VARIABLES D'INTERRUPTION ---
volatile unsigned long isrPressTime = 0;
volatile bool isrHasClicked = false;
volatile bool isrIsPressed = false;

// Fonction d'interruption matérielle
void encoderBtnISR() {
  int state = digitalRead(PIN_ENC_BTN);
  if (state == LOW) {
    isrPressTime = millis();
    isrIsPressed = true;
  } else {
    isrIsPressed = false;
    unsigned long duration = millis() - isrPressTime;
    if (duration > 5 && duration < 350) isrHasClicked = true;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  
  u8g2.begin();
  u8g2.setBusClock(400000); 

  setupControls();
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BTN), encoderBtnISR, CHANGE);
  setupAudio();
  
  bootTimer = millis(); 
}

void loop() {
  updateControls();
  
  // --- GESTION DE L'ENREGISTREMENT CONTINU ---
  if (currentState == STATE_MIC_RECORDING) {
    continueRecording(); // Écrit le son sur la carte SD en temps réel
    
    // Si les 2 secondes de la boucle sont écoulées : on coupe !
    if (millis() - recordTimer >= loopLengthMs) {
      stopRecording();
      currentState = STATE_MIC_RECORD_DONE;
      recordTimer = millis(); // On recycle le timer pour afficher le pop-up de succès
    }
  }
  
  runAudioEngine();
  
  // --- GESTION DE L'AFFICHAGE (Une seule fois !) ---
  static unsigned long lastDrawTime = 0;
  if (millis() - lastDrawTime > 33) { // 30 FPS
    lastDrawTime = millis();
    
    if (currentState == STATE_BOOT) {
      drawBootScreen();
      if (millis() - bootTimer > 1500) { 
        currentState = STATE_MAIN_MENU;
      }
    } 
    else {
      u8g2.clearBuffer();
      
      // --- LE GRAND AIGUILLAGE DES ÉCRANS ---
      if (currentState == STATE_MAIN_MENU)                 drawMainMenu();
      else if (currentState == STATE_MENU)                 drawMenuScreen();
      else if (currentState == STATE_INFO)                 drawInfoScreen();
      else if (currentState == STATE_MIC)                  drawMicScreen();
      else if (currentState == STATE_MIC_PACK)             drawMicPackScreen();
      else if (currentState == STATE_MIC_DELETE_CONFIRM)   drawMicDeleteConfirmScreen();
      else if (currentState == STATE_MIC_RECORD_READY)     drawMicRecordReadyScreen();
      else if (currentState == STATE_MIC_RECORDING)        drawMicRecordingScreen();
      else if (currentState == STATE_MIC_RECORD_DONE)      drawMicRecordDoneScreen();
      else if (currentState == STATE_LIVE)                 drawLiveScreen();
      
      // L'animation d'appui long par-dessus le reste
      if (longPressProgress > 0) {
        drawLongPressPopup();
      }

      u8g2.sendBuffer();
    }
  }

  // Quitte automatiquement l'écran "Son enregistré" après 1,5 seconde
  if (currentState == STATE_MIC_RECORD_DONE && (millis() - recordTimer > 1500)) {
    currentState = STATE_MIC_PACK;
  }
}