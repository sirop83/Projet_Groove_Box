#include "Config.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioPlaySdWav           playSdWav1;
AudioPlaySdWav           playSdWav2;
AudioPlaySdWav           playSdWav3;
AudioPlaySdWav           playSdWav4;
AudioPlaySdWav           playSdWavSystem;

AudioFilterStateVariable filter1;
AudioFilterStateVariable filter2;
AudioFilterStateVariable filter3;
AudioFilterStateVariable filter4;

AudioInputI2S            audioInput;     
AudioFilterBiquad        filtreVoix;     

AudioMixer4              mixer1;         
AudioMixer4              mixer2;         
AudioOutputI2S           i2s1;           

AudioRecordQueue         recordQueue; 
AudioConnection          patchCordRec(filtreVoix, 0, recordQueue, 0); 

AudioConnection          patchCord1(playSdWav1, 0, filter1, 0);
AudioConnection          patchCord2(playSdWav2, 0, filter2, 0);
AudioConnection          patchCord3(playSdWav3, 0, filter3, 0);
AudioConnection          patchCord4(playSdWav4, 0, filter4, 0);

AudioConnection          patchCord5(filter1, 0, mixer1, 0);
AudioConnection          patchCord6(filter2, 0, mixer1, 1);
AudioConnection          patchCord7(filter3, 0, mixer1, 2);
AudioConnection          patchCord8(filter4, 0, mixer1, 3);

AudioConnection          patchCordMic1(audioInput, 0, filtreVoix, 0);

AudioConnection          patchCordMix(mixer1, 0, mixer2, 0);      
AudioConnection          patchCordMic2(filtreVoix, 0, mixer2, 1); 
AudioConnection          patchCordSystem(playSdWavSystem, 0, mixer2, 2);

AudioConnection          patchCord9(mixer2, 0, i2s1, 0);
AudioConnection          patchCord10(mixer2, 0, i2s1, 1);

AudioControlSGTL5000     sgtl5000_1;

File recFile;
uint32_t recDataSize = 0;

// Configure les ressources de la puce audio, de la carte SD et vérifie la présence des packs personnalisés
void setupAudio() {
  AudioMemory(128); 
  
  filter1.resonance(1.5); filter2.resonance(1.5);
  filter3.resonance(1.5); filter4.resonance(1.5);
  
  sgtl5000_1.enable();
  sgtl5000_1.lineOutLevel(13);
  mixer2.gain(1, 0.0);
  mixer2.gain(2, 0.5);
  sgtl5000_1.volume(0.6);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC); 
  sgtl5000_1.micGain(30);                  
  
  filtreVoix.setHighpass(0, 150, 0.707);
  filtreVoix.setLowpass(1, 7000, 0.707);
  
  Serial.println("Initialisation de la carte SD...");
  if (!(SD.begin(BUILTIN_SDCARD))) {
    while (1) {
      Serial.println("ERREUR : Carte SD introuvable !");
      delay(1000); 
    }
  }

  Serial.println("Scan de la carte SD pour restaurer les packs...");
  for (int p = 0; p < MAX_MIC_TRACKS; p++) {
    packExists[p] = false; 
    
    for (int b = 1; b <= 8; b++) {
      char checkName[35];
      sprintf(checkName, "M_P%d_B%d.WAV", p + 1, b);
      
      if (SD.exists(checkName)) {
        packExists[p] = true; 
        break;                
      }
    }
  }
}

// Actualise dynamiquement les volumes du mixeur et gère le lancement rythmique des boucles audio
void runAudioEngine() {
  mixer1.gain(0, trackActive[0] ? trackVolumes[0] : 0.0);
  mixer1.gain(1, trackActive[1] ? trackVolumes[1] : 0.0);
  mixer1.gain(2, trackActive[2] ? trackVolumes[2] : 0.0);
  mixer1.gain(3, trackActive[3] ? trackVolumes[3] : 0.0);

  mixer2.gain(0, 1.0); 
  mixer2.gain(1, 0.0);

  if (isRunning && millis() >= nextLoopTime) {
    nextLoopTime = nextLoopTime + loopLengthMs;
    if (trackActive[0]) playTrack(0);
    if (trackActive[1]) playTrack(1);
    if (trackActive[2]) playTrack(2);
    if (trackActive[3]) playTrack(3);
  }
} 

// Coupe instantanément la lecture de tous les lecteurs SD
void stopAllAudio() {
  playSdWav1.stop(); playSdWav2.stop(); playSdWav3.stop(); playSdWav4.stop();
}

// Formate le nom du fichier ciblé et lance sa lecture sur la piste correspondante
void playTrack(int i) {
  char fileName[35]; 
  int snd = trackSound[i]; 
  
  if (isUsingMicPack) {
    sprintf(fileName, "M_P%d_B%d.WAV", activeMicPackIdx + 1, snd + 1);
    if (i == 0) playSdWav1.play(fileName);
    if (i == 1) playSdWav2.play(fileName);
    if (i == 2) playSdWav3.play(fileName);
    if (i == 3) playSdWav4.play(fileName);
  } else {
    if (snd == 0) sprintf(fileName, "%d_BASS.WAV", currentKit);
    if (snd == 1) sprintf(fileName, "%d_LEAD.WAV", currentKit);
    if (snd == 2) sprintf(fileName, "%d_PERC.WAV", currentKit);
    if (snd == 3) sprintf(fileName, "%d_BEAT.WAV", currentKit);
    if (snd == 4) sprintf(fileName, "%d_MATHYS.WAV", currentKit); 
    if (snd == 5) sprintf(fileName, "%d_LOU.WAV", currentKit);    
    if (snd == 6) sprintf(fileName, "%d_THEO.WAV", currentKit);   
    if (snd == 7) sprintf(fileName, "%d_ALLA.WAV", currentKit);   

    if (i == 0) playSdWav1.play(fileName);
    if (i == 1) playSdWav2.play(fileName);
    if (i == 2) playSdWav3.play(fileName);
    if (i == 3) playSdWav4.play(fileName);
  }
}

// Applique une fréquence de coupure au filtre passe-bas d'une piste donnée
void setTrackFilter(int track, float freq) {
  if (track == 0) filter1.frequency(freq);
  if (track == 1) filter2.frequency(freq);
  if (track == 2) filter3.frequency(freq);
  if (track == 3) filter4.frequency(freq);
}

// Initialise un nouveau fichier WAV sur la carte SD et démarre la capture du flux micro
void startRecording(const char* filename) {
  if (SD.exists(filename)) {
    SD.remove(filename); 
  }
  recFile = SD.open(filename, FILE_WRITE);
  if (recFile) {
    recDataSize = 0;
    byte header[44] = {'R','I','F','F',0,0,0,0,'W','A','V','E','f','m','t',' ',16,0,0,0,1,0,1,0,0x44,0xAC,0,0,0x88,0x58,0x01,0,2,0,16,0,'d','a','t','a',0,0,0,0};
    recFile.write(header, 44);
    recordQueue.begin(); 
  }
}

// Vide continuellement la file d'attente audio en écrivant les paquets de données dans le fichier
void continueRecording() {
  if (!recFile) return;
  
  while (recordQueue.available() >= 2) {
    byte buffer[512];
    
    memcpy(buffer, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    
    memcpy(buffer + 256, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    
    recFile.write(buffer, 512);
    recDataSize += 512;
  }
}

// Stoppe la capture et finalise l'en-tête du fichier WAV avec sa taille définitive
void stopRecording() {
  recordQueue.end(); 
  
  while (recordQueue.available() > 0) {
    recFile.write((byte*)recordQueue.readBuffer(), 256);
    recDataSize += 256;
    recordQueue.freeBuffer();
  }
  
  if (recFile) {
    uint32_t fileSize = recDataSize + 36;
    recFile.seek(4);  
    recFile.write((byte*)&fileSize, 4);
    recFile.seek(40); 
    recFile.write((byte*)&recDataSize, 4);
    recFile.close();
  }
}

// Joue le fichier sonore de démarrage
void playIntroSound() {
  playSdWavSystem.play("INTRO.WAV");
}

// Stoppe manuellement le lecteur assigné à une piste spécifique
void stopTrack(int i) {
  if (i == 0) playSdWav1.stop();
  if (i == 1) playSdWav2.stop();
  if (i == 2) playSdWav3.stop();
  if (i == 3) playSdWav4.stop();
}