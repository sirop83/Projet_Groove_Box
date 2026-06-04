#include "Config.h"

long oldEncPosition = 0;
unsigned long btnPressTime = 0;
bool isLongPress = false;

Bounce btn1 = Bounce(); Bounce btn2 = Bounce();
Bounce btn3 = Bounce(); Bounce btn4 = Bounce();

void playTrack(int i);

void setupControls() {
  pinMode(PIN_ENC_BTN, INPUT_PULLUP);
  encBtn.attach(PIN_ENC_BTN);
  encBtn.interval(10);
  
  pinMode(PIN_B1, INPUT_PULLUP); btn1.attach(PIN_B1); btn1.interval(10);
  pinMode(PIN_B2, INPUT_PULLUP); btn2.attach(PIN_B2); btn2.interval(10);
  pinMode(PIN_B3, INPUT_PULLUP); btn3.attach(PIN_B3); btn3.interval(10);
  pinMode(PIN_B4, INPUT_PULLUP); btn4.attach(PIN_B4); btn4.interval(10);

  dspValue = analogRead(PIN_POT_VOL); 
}

void resetMachine() {
  isRunning = false;
  nextLoopTime = 0;
  liveMode = SELECT_TRACK;
  selectedTrackIdx = 0;
  for (int i = 0; i < 4; i++) {
    trackActive[i] = false;
    trackVolumes[i] = 1.0; 
  }
  stopAllAudio();
}

void piloterPiste(int i) {
  trackActive[i] = !trackActive[i]; 
  
  if (!isRunning && trackActive[i]) {
    isRunning = true;
    nextLoopTime = millis() + loopLengthMs; 
    playTrack(i);
  }
  else if (isRunning) {
    bool plusAucunePisteActive = true;
    for (int j = 0; j < 4; j++) {
      if (trackActive[j]) plusAucunePisteActive = false;
    }
    if (plusAucunePisteActive) {
      isRunning = false;
      nextLoopTime = 0;
      stopAllAudio();
    }
  }
}

void updateControls() {
  encBtn.update();
  btn1.update(); btn2.update(); btn3.update(); btn4.update();

  long newEncPosition = myEnc.read() / 4;
  int deltaEnc = newEncPosition - oldEncPosition;
  if (deltaEnc != 0) oldEncPosition = newEncPosition;

  if (encBtn.fell()) {
    btnPressTime = millis();
    isLongPress = false;
  }
  
  if (encBtn.read() == LOW && (millis() - btnPressTime > 1000) && currentState == STATE_LIVE && !isLongPress) {
    isLongPress = true;
    resetMachine();
    currentState = STATE_MENU; 
    return;
  }

  if (encBtn.rose() && !isLongPress) {
    handleShortClick();
  }

  if (currentState == STATE_MENU && deltaEnc != 0) {
    currentKit += deltaEnc;
    if (currentKit > 4) currentKit = 1; // On passe de 4 à 1
    if (currentKit < 1) currentKit = 4; // On passe de 1 à 4
  }
  else if (currentState == STATE_LIVE) {
    if (btn1.fell()) piloterPiste(0);
    if (btn2.fell()) piloterPiste(1);
    if (btn3.fell()) piloterPiste(2);
    if (btn4.fell()) piloterPiste(3);

    // --- 1. ACTION DE L'ENCODEUR ROTATIF (Navigation uniquement) ---
    if (deltaEnc != 0 && liveMode == SELECT_TRACK) {
      selectedTrackIdx += deltaEnc;
      if (selectedTrackIdx > 3) selectedTrackIdx = 0;
      if (selectedTrackIdx < 0) selectedTrackIdx = 3;
    
      dspValue = analogRead(PIN_POT_VOL); 
    }

    // --- 2. ACTION DU POTENTIOMÈTRE DE VOLUME ---
    int pot = analogRead(PIN_POT_VOL);
    if (abs(pot - dspValue) > 25) {
      dspValue = pot;
      
      if (liveMode == SELECT_TRACK) {
        float masterLevel = pot / 1023.0;
        for (int j = 0; j < 4; j++) {
          trackVolumes[j] = masterLevel;
        }
      } 
      else if (liveMode == ADJUST_TRACK_VOLUME) {
        trackVolumes[selectedTrackIdx] = pot / 1023.0;
      }
      }
    // --- 3. ACTION DU POTENTIOMÈTRE DE FILTRE PASSE-BAS ---
    int potDsp = analogRead(PIN_POT_DSP);
    static int lastPotDsp = -1; 
   
    if (abs(potDsp - lastPotDsp) > 15) { 
      lastPotDsp = potDsp;
      
      float freq = 40.0 * pow((15000.0 / 40.0), (potDsp / 1023.0));
      
      updateFilter(freq);
    }
  }
}

void handleShortClick() {
  if (currentState == STATE_MENU) {
    if (currentKit == 1) currentBPM = 150.0;
    else if (currentKit == 2) currentBPM = 110.0;
    else if (currentKit == 3) currentBPM = 78.0;
    else if (currentKit == 4) currentBPM = 90.0;
    loopLengthMs = (60000.0 / currentBPM) * 16;
    
    currentState = STATE_LIVE;
    liveMode = SELECT_TRACK;
  } 
  else if (currentState == STATE_LIVE) {
    if (liveMode == SELECT_TRACK) {
      liveMode = ADJUST_TRACK_VOLUME;
    } else {
      liveMode = SELECT_TRACK;
    }
    dspValue = analogRead(PIN_POT_VOL); 
  }
}