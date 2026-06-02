#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Bounce2.h> 
#include <U8g2lib.h>

// ==========================================
// --- MACHINE À ÉTATS ---
// ==========================================
enum MachineState {
  STATE_BOOT,
  STATE_MENU,
  STATE_LIVE
};
// On force sur STATE_LIVE pour ce soir, pour vérifier que l'audio marche toujours
MachineState currentState = STATE_LIVE; 

// ==========================================
// --- OBJETS AUDIO ---
// ==========================================
AudioPlaySdWav           playWav1;
AudioPlaySdWav           playWav2;
AudioPlaySdWav           playWav3;
AudioPlaySdWav           playWav4;

AudioMixer4              mixerLeft;
AudioMixer4              mixerRight;
AudioOutputI2S           audioOut;
AudioControlSGTL5000     codec;

AudioConnection          patchCord1(playWav1, 0, mixerLeft, 0);
AudioConnection          patchCord2(playWav1, 1, mixerRight, 0);
AudioConnection          patchCord3(playWav2, 0, mixerLeft, 1);
AudioConnection          patchCord4(playWav2, 1, mixerRight, 1);
AudioConnection          patchCord5(playWav3, 0, mixerLeft, 2);
AudioConnection          patchCord6(playWav3, 1, mixerRight, 2);
AudioConnection          patchCord7(playWav4, 0, mixerLeft, 3);
AudioConnection          patchCord8(playWav4, 1, mixerRight, 3);
AudioConnection          patchCord9(mixerLeft, 0, audioOut, 0);
AudioConnection          patchCord10(mixerRight, 0, audioOut, 1);

// ==========================================
// --- HARDWARE (Boutons & Potentiomètre) ---
// ==========================================
Bounce bouton1 = Bounce(); Bounce bouton2 = Bounce(); 
Bounce bouton3 = Bounce(); Bounce bouton4 = Bounce(); 
const int pinBouton1 = 24; const int pinBouton2 = 26; 
const int pinBouton3 = 28; const int pinBouton4 = 30; 

const int pinPotentiometre = 15; 
int ancienVolume = -1;           

// ==========================================
// --- MOTEUR DU SÉQUENCEUR (DYNAMIQUE) ---
// ==========================================
int currentKit = 1;        // 1: Hip-Hop, 2: Synthwave, 3: Lo-Fi
float currentBPM = 150.0;  // Sera mis à jour selon le kit
const int BEATS_PER_LOOP = 16; 
unsigned long loopLengthMs = (60000.0 / currentBPM) * BEATS_PER_LOOP; 

bool isRunning = false;            
unsigned long nextLoopTime = 0;     
bool trackActive[4] = {false, false, false, false};

// Fonction pour charger la bonne musique selon le kit choisi
void playTrack(int i) {
  char fileName[12];
  
  if (i == 0) sprintf(fileName, "%d_BASS.WAV", currentKit);
  if (i == 1) sprintf(fileName, "%d_LEAD.WAV", currentKit);
  if (i == 2) sprintf(fileName, "%d_PERC.WAV", currentKit);
  if (i == 3) sprintf(fileName, "%d_BEAT.WAV", currentKit);

  if (i == 0) playWav1.play(fileName);
  if (i == 1) playWav2.play(fileName);
  if (i == 2) playWav3.play(fileName);
  if (i == 3) playWav4.play(fileName);
}

void stopTrack(int i) {
  if (i == 0) playWav1.stop();
  if (i == 1) playWav2.stop();
  if (i == 2) playWav3.stop();
  if (i == 3) playWav4.stop();
}

void toggleTrack(int i) {
  trackActive[i] = !trackActive[i]; 
  
  if (!isRunning && trackActive[i]) {
    isRunning = true;
    nextLoopTime = millis() + loopLengthMs; 
    playTrack(i);
  }
}

// ==========================================
// --- SETUP ---
// ==========================================
void setup() {
  Serial.begin(115200);
  AudioMemory(24); 
  
  codec.enable();
  codec.lineOutLevel(13);
  codec.volume(0.45); 

  mixerLeft.gain(0, 0);  mixerLeft.gain(1, 0);  mixerLeft.gain(2, 0);  mixerLeft.gain(3, 0);
  mixerRight.gain(0, 0); mixerRight.gain(1, 0); mixerRight.gain(2, 0); mixerRight.gain(3, 0);

  pinMode(pinBouton1, INPUT_PULLUP); bouton1.attach(pinBouton1); bouton1.interval(10); 
  pinMode(pinBouton2, INPUT_PULLUP); bouton2.attach(pinBouton2); bouton2.interval(10); 
  pinMode(pinBouton3, INPUT_PULLUP); bouton3.attach(pinBouton3); bouton3.interval(10); 
  pinMode(pinBouton4, INPUT_PULLUP); bouton4.attach(pinBouton4); bouton4.interval(10); 

  if (!(SD.begin(BUILTIN_SDCARD))) {
    while (1) {
      Serial.println("ERREUR : Carte SD introuvable !");
      delay(1000);
    }
  }
  // u8g2.begin(); // On décommentera ça demain pour l'écran
}

// ==========================================
// --- LOOP (L'AIGUILLEUR PRINCIPAL) ---
// ==========================================
void loop() {
  
  switch (currentState) {
    
    case STATE_BOOT:
      // Code de l'écran de chargement (à faire)
      currentState = STATE_MENU;
      break;

    case STATE_MENU:
      // Code de sélection de la musique avec l'encodeur (à faire)
      // Une fois sélectionné : currentState = STATE_LIVE;
      break;

    case STATE_LIVE:
      // --- 1. Gestion du Volume ---
      int valeurPotentiometre = analogRead(pinPotentiometre); 
      if (abs(valeurPotentiometre - ancienVolume) > 2) {
        float nouveauVolume = (float)valeurPotentiometre / 1023.0 * 1;
        for(int i=0; i<4; i++) {
          mixerLeft.gain(i, nouveauVolume);
          mixerRight.gain(i, nouveauVolume);
        }
        ancienVolume = valeurPotentiometre;
      }

      // --- 2. Lecture des Boutons ---
      bouton1.update(); bouton2.update(); bouton3.update(); bouton4.update();
      if (bouton1.fell()) toggleTrack(0);
      if (bouton2.fell()) toggleTrack(1);
      if (bouton3.fell()) toggleTrack(2);
      if (bouton4.fell()) toggleTrack(3);

      // --- 3. Gestion de la Boucle (Séquenceur) ---
      if (isRunning && millis() >= nextLoopTime) {
        nextLoopTime += loopLengthMs; 
        int pistesActives = 0; 
        
        for (int i = 0; i < 4; i++) {
          if (trackActive[i] == true) {
            playTrack(i);    
            pistesActives++;
          } else {
            stopTrack(i);    
          }
        }
        if (pistesActives == 0) isRunning = false;
      }
      break;
  }
}