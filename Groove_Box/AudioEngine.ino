#include "Config.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;
AudioPlaySdWav           playSdWav2;
AudioPlaySdWav           playSdWav3;
AudioPlaySdWav           playSdWav4;

// On crée 4 filtres indépendants !
AudioFilterStateVariable filter1;
AudioFilterStateVariable filter2;
AudioFilterStateVariable filter3;
AudioFilterStateVariable filter4;

AudioMixer4              mixer1;         
AudioOutputI2S           i2s1;           

// 1. Les pistes rentrent dans leurs filtres respectifs
AudioConnection          patchCord1(playSdWav1, 0, filter1, 0);
AudioConnection          patchCord2(playSdWav2, 0, filter2, 0);
AudioConnection          patchCord3(playSdWav3, 0, filter3, 0);
AudioConnection          patchCord4(playSdWav4, 0, filter4, 0);

// 2. Les filtres rentrent dans le mixeur
AudioConnection          patchCord5(filter1, 0, mixer1, 0);
AudioConnection          patchCord6(filter2, 0, mixer1, 1);
AudioConnection          patchCord7(filter3, 0, mixer1, 2);
AudioConnection          patchCord8(filter4, 0, mixer1, 3);

// 3. Le mixeur sort vers le casque
AudioConnection          patchCord9(mixer1, 0, i2s1, 0);
AudioConnection          patchCord10(mixer1, 0, i2s1, 1);

AudioControlSGTL5000     sgtl5000_1;     
// GUItool: end automatically generated code

void setupAudio() {
  AudioMemory(64); 
  
  filter1.resonance(1.5);
  filter2.resonance(1.5);
  filter3.resonance(1.5);
  filter4.resonance(1.5);
  
  sgtl5000_1.enable();
  sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.volume(0.6); 

  Serial.println("Initialisation de la carte SD...");
  if (!(SD.begin(BUILTIN_SDCARD))) {
    while (1) {
      Serial.println("ERREUR : Carte SD introuvable !");
      delay(1000); 
    }
  }
}

void runAudioEngine() {
  // Gestion instantanée du volume et du Mute/Unmute de chaque piste
  mixer1.gain(0, trackActive[0] ? trackVolumes[0] : 0.0);
  mixer1.gain(1, trackActive[1] ? trackVolumes[1] : 0.0);
  mixer1.gain(2, trackActive[2] ? trackVolumes[2] : 0.0);
  mixer1.gain(3, trackActive[3] ? trackVolumes[3] : 0.0);
  // SÉQUENCEUR
  if (isRunning && millis() >= nextLoopTime) {
    nextLoopTime = nextLoopTime + loopLengthMs;

    if (trackActive[0]) playTrack(0);
    if (trackActive[1]) playTrack(1);
    if (trackActive[2]) playTrack(2);
    if (trackActive[3]) playTrack(3);
  }
} 

void stopAllAudio() {
  playSdWav1.stop();
  playSdWav2.stop();
  playSdWav3.stop();
  playSdWav4.stop();
}

void playTrack(int i) {
  char fileName[15]; 
  
  if (i == 0) sprintf(fileName, "%d_BASS.WAV", currentKit);
  if (i == 1) sprintf(fileName, "%d_LEAD.WAV", currentKit);
  if (i == 2) sprintf(fileName, "%d_PERC.WAV", currentKit);
  if (i == 3) sprintf(fileName, "%d_BEAT.WAV", currentKit);

  if (i == 0) playSdWav1.play(fileName);
  if (i == 1) playSdWav2.play(fileName);
  if (i == 2) playSdWav3.play(fileName);
  if (i == 3) playSdWav4.play(fileName);
}
void setTrackFilter(int track, float freq) {
  if (track == 0) filter1.frequency(freq);
  if (track == 1) filter2.frequency(freq);
  if (track == 2) filter3.frequency(freq);
  if (track == 3) filter4.frequency(freq);
}