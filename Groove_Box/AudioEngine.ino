#include "Config.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// --- 1. OBJETS AUDIO ---
AudioPlaySdWav           playSdWav1;
AudioPlaySdWav           playSdWav2;
AudioPlaySdWav           playSdWav3;
AudioPlaySdWav           playSdWav4;

AudioFilterStateVariable filter1;
AudioFilterStateVariable filter2;
AudioFilterStateVariable filter3;
AudioFilterStateVariable filter4;

AudioInputI2S            audioInput;     // LE MICRO
AudioFilterBiquad        filtreVoix;     // LE FILTRE DU MICRO

AudioMixer4              mixer1;         // Mixeur 1 : Mélange les 4 pistes
AudioMixer4              mixer2;         // Mixeur 2 : Mélange le Mixer 1 + le Micro
AudioOutputI2S           i2s1;           // Sortie audio

AudioRecordQueue         recordQueue; // L'outil qui capture le flux micro
AudioConnection          patchCordRec(filtreVoix, 0, recordQueue, 0); // Connecté après le filtre voix

// --- 2. CÂBLAGE ---
// Pistes vers Filtres
AudioConnection          patchCord1(playSdWav1, 0, filter1, 0);
AudioConnection          patchCord2(playSdWav2, 0, filter2, 0);
AudioConnection          patchCord3(playSdWav3, 0, filter3, 0);
AudioConnection          patchCord4(playSdWav4, 0, filter4, 0);

// Filtres vers Mixer 1
AudioConnection          patchCord5(filter1, 0, mixer1, 0);
AudioConnection          patchCord6(filter2, 0, mixer1, 1);
AudioConnection          patchCord7(filter3, 0, mixer1, 2);
AudioConnection          patchCord8(filter4, 0, mixer1, 3);

// Micro vers Filtre Micro
AudioConnection          patchCordMic1(audioInput, 0, filtreVoix, 0);

// Mixer 1 ET Filtre Micro vers Mixer 2
AudioConnection          patchCordMix(mixer1, 0, mixer2, 0);      // Toutes les pistes entrent ici
AudioConnection          patchCordMic2(filtreVoix, 0, mixer2, 1); // Le micro entre ici

// Mixer 2 vers Sortie
AudioConnection          patchCord9(mixer2, 0, i2s1, 0);
AudioConnection          patchCord10(mixer2, 0, i2s1, 1);

AudioControlSGTL5000     sgtl5000_1;

void setupAudio() {
  AudioMemory(128); 
  
  filter1.resonance(1.5); filter2.resonance(1.5);
  filter3.resonance(1.5); filter4.resonance(1.5);
  
  // --- CONFIG MICRO ---
  sgtl5000_1.enable();
  sgtl5000_1.lineOutLevel(13);
  mixer2.gain(1, 0.0);
  sgtl5000_1.volume(0.6);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC); // Active le micro
  sgtl5000_1.micGain(30);                  // Gain du micro
  
  filtreVoix.setHighpass(0, 150, 0.707);
  filtreVoix.setLowpass(1, 7000, 0.707);
  
  Serial.println("Initialisation de la carte SD...");
  if (!(SD.begin(BUILTIN_SDCARD))) {
    while (1) {
      Serial.println("ERREUR : Carte SD introuvable !");
      delay(1000); 
    }
  }

  // --- NOUVEAUTÉ : RECONSTRUCTION AUTOMATIQUE DU MENU AU DÉMARRAGE ---
  Serial.println("Scan de la carte SD pour restaurer les packs...");
  for (int p = 0; p < MAX_MIC_TRACKS; p++) {
    packExists[p] = false; // Par défaut, on l'imagine vide
    
    // On vérifie si au moins un des 4 boutons possède un enregistrement
    for (int b = 1; b <= 4; b++) {
      char checkName[35];
      sprintf(checkName, "M_P%d_B%d.WAV", p + 1, b);
      
      if (SD.exists(checkName)) {
        packExists[p] = true; // Un fichier existe ! On active le pack dans le menu
        break;                // Pas besoin de vérifier les autres boutons de ce pack, on passe au pack suivant
      }
    }
  }
}

void runAudioEngine() {
  // Volume des 4 pistes sur mixer1
  mixer1.gain(0, trackActive[0] ? trackVolumes[0] : 0.0);
  mixer1.gain(1, trackActive[1] ? trackVolumes[1] : 0.0);
  mixer1.gain(2, trackActive[2] ? trackVolumes[2] : 0.0);
  mixer1.gain(3, trackActive[3] ? trackVolumes[3] : 0.0);

  // Volume du micro sur mixer2
  mixer2.gain(0, 1.0); // On laisse toujours passer la musique
  // mixer2.gain(1, (currentState == STATE_MIC) ? 3.0 : 0.0); // Le micro s'allume que dans STATE_MIC
  mixer2.gain(1, 0.0);

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
  playSdWav1.stop(); playSdWav2.stop(); playSdWav3.stop(); playSdWav4.stop();
}

void playTrack(int i) {
  char fileName[35]; 
  
  if (isUsingMicPack) {
    // Si on lit un pack micro personnalisé
    sprintf(fileName, "M_P%d_B%d.WAV", activeMicPackIdx + 1, i + 1);
    if (i == 0) playSdWav1.play(fileName);
    if (i == 1) playSdWav2.play(fileName);
    if (i == 2) playSdWav3.play(fileName);
    if (i == 3) playSdWav4.play(fileName);
  } else {
    // Style d'usine classique
    if (i == 0) sprintf(fileName, "%d_BASS.WAV", currentKit);
    if (i == 1) sprintf(fileName, "%d_LEAD.WAV", currentKit);
    if (i == 2) sprintf(fileName, "%d_PERC.WAV", currentKit);
    if (i == 3) sprintf(fileName, "%d_BEAT.WAV", currentKit);

    if (i == 0) playSdWav1.play(fileName);
    if (i == 1) playSdWav2.play(fileName);
    if (i == 2) playSdWav3.play(fileName);
    if (i == 3) playSdWav4.play(fileName);
  }
}

void setTrackFilter(int track, float freq) {
  if (track == 0) filter1.frequency(freq);
  if (track == 1) filter2.frequency(freq);
  if (track == 2) filter3.frequency(freq);
  if (track == 3) filter4.frequency(freq);
}

File recFile;
uint32_t recDataSize = 0;

void startRecording(const char* filename) {
  if (SD.exists(filename)) {
    SD.remove(filename); // Supprime l'ancien enregistrement s'il existe
  }
  recFile = SD.open(filename, FILE_WRITE);
  if (recFile) {
    recDataSize = 0;
    // Écriture d'un en-tête WAV vide de 44 octets (les tailles seront corrigées à la fin)
    byte header[44] = {'R','I','F','F',0,0,0,0,'W','A','V','E','f','m','t',' ',16,0,0,0,1,0,1,0,0x44,0xAC,0,0,0x88,0x58,0x01,0,2,0,16,0,'d','a','t','a',0,0,0,0};
    recFile.write(header, 44);
    recordQueue.begin(); // Démarre la capture
  }
}

void continueRecording() {
  if (!recFile) return;
  
  // LE SECRET EST ICI : "while" au lieu de "if" !
  // On boucle jusqu'à ce que la file d'attente soit complètement vide.
  while (recordQueue.available() >= 2) {
    byte buffer[512];
    
    // 1. On attrape le premier paquet de 256 octets
    memcpy(buffer, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    
    // 2. On attrape le deuxième paquet et on le colle à la suite
    memcpy(buffer + 256, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    
    // 3. On sauvegarde le gros paquet consolidé sur la carte SD
    recFile.write(buffer, 512);
    recDataSize += 512;
  }
}

void stopRecording() {
  recordQueue.end(); // Arrête la capture du micro
  
  // On vide les derniers petits morceaux de 256 octets qui pourraient rester coincés
  while (recordQueue.available() > 0) {
    recFile.write((byte*)recordQueue.readBuffer(), 256);
    recDataSize += 256;
    recordQueue.freeBuffer();
  }
  
  if (recFile) {
    // On met à jour l'en-tête du fichier WAV pour que la taille soit exacte
    uint32_t fileSize = recDataSize + 36;
    recFile.seek(4);  
    recFile.write((byte*)&fileSize, 4);
    recFile.seek(40); 
    recFile.write((byte*)&recDataSize, 4);
    recFile.close();
  }
}