#include "Config.h"

long oldEncPosition = 0;
unsigned long btnPressTime = 0;
bool isLongPress = false;

// Variables pour les 4 boutons de piste
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
}

// Fonction intermédiaire pour gérer l'activation propre d'une piste
void piloterPiste(int i) {
  trackActive[i] = !trackActive[i]; 
  
  // Si le séquenceur est arrêté et qu'on active une piste, on le réveille immédiatement
  if (!isRunning && trackActive[i]) {
    Serial.println(">>> DEMARRAGE DU SEQUENCEUR ! <<<");
    isRunning = true;
    nextLoopTime = millis() + loopLengthMs; 
    playTrack(i); // On lance le premier sample immédiatement
  }
}

void updateControls() {
  encBtn.update();
  btn1.update(); btn2.update(); btn3.update(); btn4.update();

  long newEncPosition = myEnc.read() / 4;
  int deltaEnc = newEncPosition - oldEncPosition;
  if (deltaEnc != 0) oldEncPosition = newEncPosition;

  // --- GESTION DU CLIC ET APPUI LONG ---
  if (encBtn.fell()) {
    btnPressTime = millis();
    isLongPress = false;
  }
  
  if (encBtn.read() == LOW && (millis() - btnPressTime > 1000) && currentState == STATE_LIVE && !isLongPress) {
    isLongPress = true;
    currentState = STATE_MENU; 
    return;
  }

  if (encBtn.rose() && !isLongPress) {
    handleShortClick();
  }

  // --- LOGIQUE SELON L'ÉTAT ---
  if (currentState == STATE_MENU && deltaEnc != 0) {
    currentKit += deltaEnc;
    if (currentKit > 3) currentKit = 1;
    if (currentKit < 1) currentKit = 3;
  } 
  else if (currentState == STATE_LIVE) {
    // Utilisation de la nouvelle fonction pour inclure le démarrage immédiat
    if (btn1.fell()) piloterPiste(0);
    if (btn2.fell()) piloterPiste(1);
    if (btn3.fell()) piloterPiste(2);
    if (btn4.fell()) piloterPiste(3);

    if (deltaEnc != 0) {
      if (liveMode == SELECT_TRACK) {
        selectedTrackIdx += deltaEnc;
        if (selectedTrackIdx > 3) selectedTrackIdx = 0;
        if (selectedTrackIdx < 0) selectedTrackIdx = 3;
      } else if (liveMode == ADJUST_TRACK_VOLUME) {
        trackVolumes[selectedTrackIdx] += (deltaEnc * 0.1); 
        if (trackVolumes[selectedTrackIdx] > 1.0) trackVolumes[selectedTrackIdx] = 1.0;
        if (trackVolumes[selectedTrackIdx] < 0.0) trackVolumes[selectedTrackIdx] = 0.0;
      }
    }

    int pot = analogRead(PIN_POT_DSP);
    if (abs(pot - dspValue) > 10) {
      dspValue = pot;
      showDspPopUp = true;
      dspPopUpTimer = millis();
    }
    if (showDspPopUp && (millis() - dspPopUpTimer > 2000)) {
      showDspPopUp = false; 
    }
  }
}

void handleShortClick() {
  if (currentState == STATE_MENU) {
    if (currentKit == 1) currentBPM = 150.0;
    else if (currentKit == 2) currentBPM = 110.0;
    else if (currentKit == 3) currentBPM = 78.0;
    loopLengthMs = (60000.0 / currentBPM) * 16;
    
    currentState = STATE_LIVE;
    liveMode = SELECT_TRACK;
  } 
  else if (currentState == STATE_LIVE) {
    if (liveMode == SELECT_TRACK) liveMode = ADJUST_TRACK_VOLUME;
    else liveMode = SELECT_TRACK;
  }
}