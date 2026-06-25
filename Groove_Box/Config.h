#ifndef CONFIG_H
#define CONFIG_H

#include <U8g2lib.h>
#include <Bounce2.h>
#include <Encoder.h> // Bibliothèque standard Teensy pour l'encodeur

// --- ÉTATS GLOBAUX ---
enum MachineState { 
  STATE_BOOT, STATE_MAIN_MENU, STATE_MENU, STATE_INFO, STATE_LIVE, 
  STATE_MIC, STATE_MIC_PACK, STATE_MIC_DELETE_CONFIRM,
  STATE_MIC_RECORD_READY, STATE_MIC_RECORDING, STATE_MIC_RECORD_DONE 
};
extern MachineState currentState;
extern int mainMenuSelection;

// --- VARIABLES DU MICRO ---
#define MAX_MIC_TRACKS 4      
extern bool packExists[MAX_MIC_TRACKS];
extern int micMenuSelection;  
extern int selectedMicPackIdx;   // <- Retient le pack sélectionné (0 à 3)
extern int micPackMenuSelection; // <- Position du curseur dans le sous-menu (0 à 2)
extern int micDeleteConfirmSelection;
extern int chosenRecordBtn;       // <- Retient le bouton physique choisi (0 à 3)
extern unsigned long recordTimer; // <- Timer pour la barre de progression
extern bool isUsingMicPack;       // <- Permet de savoir si on joue vos enregistrements ou les styles d'usine
extern int activeMicPackIdx;      // <- Retient quel pack personnalisé est actif en lecture

// --- FONCTIONS AUDIO ENREGISTREMENT ---
void startRecording(const char* filename);
void continueRecording();
void stopRecording();

// --- SOUS-ÉTATS DU MODE LIVE ---
enum LiveSubState { SELECT_TRACK, ADJUST_TRACK_VOLUME};
extern LiveSubState liveMode;

// --- PINS (À adapter selon le câblage) ---
const int PIN_B1 = 30; const int PIN_B2 = 28;
const int PIN_B3 = 26; const int PIN_B4 = 27;

const int PIN_B5 = 25; const int PIN_B6 = 29;
const int PIN_B7 = 31; const int PIN_B8 = 32; 

const int PIN_LED = 38;

const int PIN_POT_DSP = 16;  
const int PIN_POT_VOL = 14;  
const int PIN_ENC_A = 39;     
const int PIN_ENC_B = 37;     
const int PIN_ENC_BTN = 41;   

// --- VARIABLES DU JEU ---
extern int currentKit;       // 1: Hip-Hop, 2: Synthwave, 3: Lo-Fi
extern float currentBPM;
extern unsigned long loopLengthMs;
extern bool trackActive[4];
extern float trackVolumes[4]; 
extern float trackFilters[4];
extern int trackSound[4];
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
void playIntroSound();
void setupControls();
extern unsigned long bootTimer;
void stopTrack(int i);

extern int longPressProgress;
void drawLongPressPopup();

void stopAllAudio();
void resetMachine();

#endif

