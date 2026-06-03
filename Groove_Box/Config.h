#ifndef CONFIG_H
#define CONFIG_H

#include <U8g2lib.h>
#include <Bounce2.h>
#include <Encoder.h> // Bibliothèque standard Teensy pour l'encodeur

// --- ÉTATS GLOBAUX ---
enum MachineState { STATE_BOOT, STATE_MENU, STATE_LIVE };
extern MachineState currentState;

// --- SOUS-ÉTATS DU MODE LIVE ---
enum LiveSubState { SELECT_TRACK, ADJUST_TRACK_VOLUME };
extern LiveSubState liveMode;

// --- PINS (À adapter selon votre câblage) ---
const int PIN_B1 = 24; const int PIN_B2 = 26;
const int PIN_B3 = 28; const int PIN_B4 = 30;
const int PIN_POT_DSP = 14;  
const int PIN_POT_VOL = 15;  
const int PIN_ENC_A = 2;     
const int PIN_ENC_B = 3;     
const int PIN_ENC_BTN = 4;   

// --- VARIABLES DU JEU ---
extern int currentKit;       // 1: Hip-Hop, 2: Synthwave, 3: Lo-Fi
extern float currentBPM;
extern unsigned long loopLengthMs;
extern bool trackActive[4];
extern float trackVolumes[4]; 
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

#endif

