#include "Config.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav3;     //xy=264,171
AudioPlaySdWav           playSdWav4;     //xy=265,239
AudioPlaySdWav           playSdWav1;     //xy=266,66
AudioPlaySdWav           playSdWav2;     //xy=266,120
AudioMixer4              mixer1;         //xy=523,75
AudioFilterStateVariable filter1;        //xy=681,72
AudioOutputI2S           i2s1;           //xy=883,69
AudioConnection          patchCord1(playSdWav3, 0, mixer1, 2);
AudioConnection          patchCord2(playSdWav4, 0, mixer1, 3);
AudioConnection          patchCord3(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord4(playSdWav2, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, filter1, 0);
AudioConnection          patchCord6(filter1, 0, i2s1, 0);
AudioConnection          patchCord7(filter1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=128,112
// GUItool: end automatically generated code


void setupAudio() {
  AudioMemory(24); 
  
  sgtl5000_1.enable();
  sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.volume(0.45); // Sécurité de base

  // On initialise l'unique mixer1 à 0 pour éviter un gros "CLAC" au démarrage
  mixer1.gain(0, 0);  
  mixer1.gain(1, 0);  
  mixer1.gain(2, 0);  
  mixer1.gain(3, 0);

  Serial.println("Initialisation de la carte SD...");
  if (!(SD.begin(BUILTIN_SDCARD))) {
    while (1) {
      Serial.println("ERREUR : Carte SD introuvable !");
      delay(1000); // Fait clignoter la console si la carte est mal enfoncée
    }
  }
  Serial.println("Systeme pret. La machine a etats prend le relais !");
}

void runAudioEngine() {
  // Cette fonction est appelée en boucle quand on est dans STATE_LIVE
  
  if (isRunning && millis() >= nextLoopTime) {
    nextLoopTime = millis() + loopLengthMs; 
    
    // Au début de chaque boucle, on relance les pistes qui sont actives (Unmute)
    if (trackActive[0]) playTrack(0);
    if (trackActive[1]) playTrack(1);
    if (trackActive[2]) playTrack(2);
    if (trackActive[3]) playTrack(3);
  }
}

void playTrack(int i) {
  char fileName[12]; 
  
  // Le "%d" est remplacé par le numéro du kit (1, 2 ou 3)
  if (i == 0) sprintf(fileName, "%d_BASS.WAV", currentKit);
  if (i == 1) sprintf(fileName, "%d_LEAD.WAV", currentKit);
  if (i == 2) sprintf(fileName, "%d_PERC.WAV", currentKit);
  if (i == 3) sprintf(fileName, "%d_BEAT.WAV", currentKit);

  // Lancement du fichier audio sur la bonne piste (avec les bons noms générés par le GUI)
  if (i == 0) playSdWav1.play(fileName);
  if (i == 1) playSdWav2.play(fileName);
  if (i == 2) playSdWav3.play(fileName);
  if (i == 3) playSdWav4.play(fileName);
}