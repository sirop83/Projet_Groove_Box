#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputI2S            audioInput;
AudioFilterBiquad        filtreVoix;     // Le filtre pour nettoyer le son
AudioMixer4              mixer1;
AudioOutputI2S           audioOutput;

// Câblage virtuel : Micro -> Filtre -> Mixeur -> Sorties G/D
AudioConnection          patchCord1(audioInput, 0, filtreVoix, 0); 
AudioConnection          patchCord2(filtreVoix, 0, mixer1, 0);
AudioConnection          patchCord3(mixer1, 0, audioOutput, 0);
AudioConnection          patchCord4(mixer1, 0, audioOutput, 1);

AudioControlSGTL5000     sgtl5000_1;

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  AudioMemory(30); 
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.4); 

  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  
  // Gain matériel modéré pour éviter de capter le bruit de fond de l'USB
  sgtl5000_1.micGain(40); 
  mixer1.gain(0, 1.2);    // On booste proprement dans le mixeur numérique

  // --- CONFIGURATION DU FILTRE ---
  // Étape A : On coupe les infra-graves sous 150Hz (pops, bruits de table)
  filtreVoix.setHighpass(0, 150, 0.707);
  // Étape B : On coupe les aigus stridents au-dessus de 7000Hz (le souffle)
  filtreVoix.setLowpass(1, 7000, 0.707);

  Serial.println("Test audio branché sur filtre : Prêt !");
}

void loop() {
  // Le traitement audio se fait automatiquement en tâche de fond
}
