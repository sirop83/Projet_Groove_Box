#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Bounce2.h> 

// --- OBJETS AUDIO ---
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

// --- CONFIGURATION MATÉRIELLE BOUTONS & POTENTIOMÈTRE ---
Bounce bouton1 = Bounce(); Bounce bouton2 = Bounce(); 
Bounce bouton3 = Bounce(); Bounce bouton4 = Bounce(); 
const int pinBouton1 = 24; const int pinBouton2 = 26; 
const int pinBouton3 = 28; const int pinBouton4 = 30; 

const int pinPotentiometre = 15; // Broche pour le volume (Trou "VOL")
int ancienVolume = -1;           // Mémoire pour le filtre anti-spam

// ==========================================
// --- MOTEUR DU SÉQUENCEUR (LE CHEF D'ORCHESTRE) ---
// ==========================================
const float BPM = 150.0;
const int BEATS_PER_LOOP = 16; // 4 mesures de 4 temps
const unsigned long LOOP_LENGTH_MS = (60000.0 / BPM) * BEATS_PER_LOOP; 

bool isRunning = false;             
unsigned long nextLoopTime = 0;     
bool trackActive[4] = {false, false, false, false};

void playTrack(int i) {
  if (i == 0) playWav1.play("1_BASS.WAV");
  if (i == 1) playWav2.play("1_LEAD.WAV");
  if (i == 2) playWav3.play("1_PERC.WAV");
  if (i == 3) playWav4.play("1_BEAT.WAV");
}

void stopTrack(int i) {
  if (i == 0) playWav1.stop();
  if (i == 1) playWav2.stop();
  if (i == 2) playWav3.stop();
  if (i == 3) playWav4.stop();
}

void toggleTrack(int i) {
  trackActive[i] = !trackActive[i]; 

  if (trackActive[i]) {
    Serial.print("Piste "); Serial.print(i+1); Serial.println(" : MISE EN ATTENTE (Play)");
  } else {
    Serial.print("Piste "); Serial.print(i+1); Serial.println(" : MISE EN ATTENTE (Stop)");
  }

  if (!isRunning && trackActive[i]) {
    Serial.println(">>> DEMARRAGE DU SEQUENCEUR ! <<<");
    isRunning = true;
    nextLoopTime = millis() + LOOP_LENGTH_MS; 
    playTrack(i);
  }
}
// ==========================================

void setup() {
  Serial.begin(115200);
  AudioMemory(24); 
  
  codec.enable();
  codec.lineOutLevel(13);
  codec.volume(0.45); // Pour la sécurité de la puce, on laisse un volume de base

  // On initialise les mixers à 0 (le potentiomètre prendra le relais tout de suite dans le loop)
  mixerLeft.gain(0, 0);  mixerLeft.gain(1, 0);  mixerLeft.gain(2, 0);  mixerLeft.gain(3, 0);
  mixerRight.gain(0, 0); mixerRight.gain(1, 0); mixerRight.gain(2, 0); mixerRight.gain(3, 0);

  pinMode(pinBouton1, INPUT_PULLUP); bouton1.attach(pinBouton1); bouton1.interval(10); 
  pinMode(pinBouton2, INPUT_PULLUP); bouton2.attach(pinBouton2); bouton2.interval(10); 
  pinMode(pinBouton3, INPUT_PULLUP); bouton3.attach(pinBouton3); bouton3.interval(10); 
  pinMode(pinBouton4, INPUT_PULLUP); bouton4.attach(pinBouton4); bouton4.interval(10); 

  Serial.println("Initialisation de la carte SD...");
  if (!(SD.begin(BUILTIN_SDCARD))) {
    while (1) {
      Serial.println("ERREUR : Carte SD introuvable !");
      delay(1000);
    }
  }
  Serial.println("Systeme pret. Appuyez sur un bouton pour lancer la boucle !");
}

void loop() {
  // --- NOUVEAU : GESTION DU VOLUME VIA LES MIXERS ---
  int valeurPotentiometre = analogRead(pinPotentiometre); 
  
  // Filtre anti-spam : on ne modifie le son que si on a vraiment tourné le bouton
  if (abs(valeurPotentiometre - ancienVolume) > 2) {
    
    // Calcul du volume (de 0.0 à 0.8)
    float nouveauVolume = (float)valeurPotentiometre / 1023.0 * 1;
    
    // On applique ce volume DIRECTEMENT sur les sorties Line Out via les Mixers
    mixerLeft.gain(0, nouveauVolume);
    mixerLeft.gain(1, nouveauVolume);
    mixerLeft.gain(2, nouveauVolume);
    mixerLeft.gain(3, nouveauVolume);
    
    mixerRight.gain(0, nouveauVolume);
    mixerRight.gain(1, nouveauVolume);
    mixerRight.gain(2, nouveauVolume);
    mixerRight.gain(3, nouveauVolume);
    
    ancienVolume = valeurPotentiometre;
    
    // Affichage pour vérifier que ça marche bien
    Serial.print("Volume modifie : ");
    Serial.println(nouveauVolume);
  }
  // ----------------------------------------------------

  bouton1.update(); bouton2.update(); bouton3.update(); bouton4.update();

  // 1. Lecture des boutons
  if (bouton1.fell()) toggleTrack(0);
  if (bouton2.fell()) toggleTrack(1);
  if (bouton3.fell()) toggleTrack(2);
  if (bouton4.fell()) toggleTrack(3);

  // 2. Vérification du temps
  if (isRunning && millis() >= nextLoopTime) {
    nextLoopTime += LOOP_LENGTH_MS; 
    Serial.println("--- DEBUT DE LA NOUVELLE BOUCLE ---");

    int pistesActives = 0; 
    
    for (int i = 0; i < 4; i++) {
      if (trackActive[i] == true) {
        playTrack(i);     
        pistesActives++;
      } else {
        stopTrack(i);     
      }
    }

    if (pistesActives == 0) {
      isRunning = false;
      Serial.println(">>> SEQUENCEUR ARRETE <<<");
    }
  }
}