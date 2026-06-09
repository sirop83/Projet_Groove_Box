#include "Config.h"

long oldEncPosition = 0;
unsigned long btnPressTime = 0;
bool isLongPress = false;

Bounce btn1 = Bounce();
Bounce btn2 = Bounce();
Bounce btn3 = Bounce(); 
Bounce btn4 = Bounce();

void setupControls() {
  pinMode(PIN_ENC_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BTN), encoderBtnISR, CHANGE);
  
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
    trackVolumes[i] = 0.5; 
    trackFilters[i] = 15000.0;
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
  btn1.update(); btn2.update(); btn3.update(); btn4.update();

  if (isrHasClicked) {
    isrHasClicked = false;
    longPressProgress = 0;
    if (!isLongPress) handleShortClick();
  }

  if (isrIsPressed) {
    unsigned long holdTime = millis() - isrPressTime;
    if (holdTime > 350 && !isLongPress) {
      longPressProgress = map(holdTime, 350, 800, 0, 100);
      if (holdTime >= 800) {
        isLongPress = true;
        longPressProgress = 0;
        if (currentState == STATE_LIVE) { resetMachine(); currentState = STATE_MENU; }
        else if (currentState == STATE_MENU) currentState = STATE_MAIN_MENU;
        else if (currentState == STATE_INFO) currentState = STATE_MAIN_MENU;
        else if (currentState == STATE_MIC) currentState = STATE_MAIN_MENU;
      }
    }
  } else {
    isLongPress = false;
    longPressProgress = 0;
  }

  long rawEnc = myEnc.read();
  int deltaEnc = 0;
  if (rawEnc >= oldEncPosition + 4) { deltaEnc = 1; oldEncPosition = rawEnc; }
  else if (rawEnc <= oldEncPosition - 4) { deltaEnc = -1; oldEncPosition = rawEnc; }

  if (currentState == STATE_MAIN_MENU && deltaEnc != 0) {
    mainMenuSelection += deltaEnc;
    if (mainMenuSelection > 2) mainMenuSelection = 0;
    if (mainMenuSelection < 0) mainMenuSelection = 2;
  }

  if (currentState == STATE_MENU && deltaEnc != 0) {
    currentKit += deltaEnc;
    if (currentKit > 5) currentKit = 1; 
    if (currentKit < 1) currentKit = 5; 
  }
  else if (currentState == STATE_LIVE) {
    if (btn1.fell()) piloterPiste(0);
    if (btn2.fell()) piloterPiste(1);
    if (btn3.fell()) piloterPiste(2);
    if (btn4.fell()) piloterPiste(3);

    if (deltaEnc != 0 && liveMode == SELECT_TRACK) {
      selectedTrackIdx += deltaEnc;
      if (selectedTrackIdx > 3) selectedTrackIdx = 0;
      if (selectedTrackIdx < 0) selectedTrackIdx = 3;
      dspValue = analogRead(PIN_POT_VOL);
    }

    int pot = analogRead(PIN_POT_VOL);
    if (abs(pot - dspValue) > 25) {
      dspValue = pot;
      if (liveMode == SELECT_TRACK) {
        float masterLevel = pot / 1023.0;
        for (int j = 0; j < 4; j++) trackVolumes[j] = masterLevel;
      } 
      else if (liveMode == ADJUST_TRACK_VOLUME) {
        trackVolumes[selectedTrackIdx] = pot / 1023.0;
      }
    }
    
    int potDsp = analogRead(PIN_POT_DSP);
    static int lastPotDsp = -1; 
    if (abs(potDsp - lastPotDsp) > 15) { 
      lastPotDsp = potDsp;
      float freq = 40.0 * pow((15000.0 / 40.0), (potDsp / 1023.0));
      if (liveMode == SELECT_TRACK) {
        for (int j = 0; j < 4; j++) { trackFilters[j] = freq; setTrackFilter(j, freq); }
      } 
      else if (liveMode == ADJUST_TRACK_VOLUME) {
        trackFilters[selectedTrackIdx] = freq;
        setTrackFilter(selectedTrackIdx, freq);
      }
    }
  }
}

void handleShortClick() {
  if (currentState == STATE_MAIN_MENU) {
    if (mainMenuSelection == 0) {
      currentState = STATE_MENU;      
    }
    else if (mainMenuSelection == 1) {
      currentState = STATE_MIC;       
    }
    else if (mainMenuSelection == 2) {
      currentState = STATE_INFO;      
    }
  }
  else if (currentState == STATE_MENU) {
    if (currentKit == 1) currentBPM = 150.0;
    else if (currentKit == 2) currentBPM = 110.0;
    else if (currentKit == 3) currentBPM = 78.0;
    else if (currentKit == 4) currentBPM = 90.0;
    else if (currentKit == 5) currentBPM = 120.0; 
    
    loopLengthMs = (60000.0 / currentBPM) * 16;
    currentState = STATE_LIVE;
    liveMode = SELECT_TRACK;
  } 
  else if (currentState == STATE_LIVE) {
    if (liveMode == SELECT_TRACK) liveMode = ADJUST_TRACK_VOLUME;
    else liveMode = SELECT_TRACK;
    dspValue = analogRead(PIN_POT_VOL); 
  }
}