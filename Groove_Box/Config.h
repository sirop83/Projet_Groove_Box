#ifndef CONFIG_H
#define CONFIG_H

#include <U8g2lib.h>
#include <Bounce2.h>
#include <Encoder.h> // Bibliothèque standard Teensy pour l'encodeur

// --- ÉTATS GLOBAUX ---
enum MachineState { STATE_BOOT, STATE_MAIN_MENU, STATE_MENU, STATE_INFO, STATE_LIVE, STATE_MIC };
extern MachineState currentState;
extern int mainMenuSelection;

// --- SOUS-ÉTATS DU MODE LIVE ---
enum LiveSubState { SELECT_TRACK, ADJUST_TRACK_VOLUME };
extern LiveSubState liveMode;

// --- PINS (À adapter selon le câblage) ---
const int PIN_B1 = 30; const int PIN_B2 = 28;
const int PIN_B3 = 26; const int PIN_B4 = 24;
const int PIN_POT_DSP = 14;  
const int PIN_POT_VOL = 16;  
const int PIN_ENC_A = 37;     
const int PIN_ENC_B = 39;     
const int PIN_ENC_BTN = 41;   

// --- VARIABLES DU JEU ---
extern int currentKit;       // 1: Hip-Hop, 2: Synthwave, 3: Lo-Fi
extern float currentBPM;
extern unsigned long loopLengthMs;
extern bool trackActive[4];
extern float trackVolumes[4]; 
extern float trackFilters[4];
extern int selectedTrackIdx;  
extern unsigned long nextLoopTime;

// --- POP-UP DSP ---
extern unsigned long dspPopUpTimer;
extern bool showDspPopUp;
extern int dspValue; 

// --- OBJETS GLOBAUX ---
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Encoder myEnc;
extern Bounce encBtn;
extern bool isRunning;
void playTrack(int i);
void drawBootScreen();
void setupControls();
extern unsigned long bootTimer;


extern int longPressProgress;
void drawLongPressPopup();

void stopAllAudio();
void resetMachine();

#endif

