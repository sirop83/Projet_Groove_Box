#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <Bounce2.h> 

#define AMPLITUDE (0.5)

// --- CONFIGURATION DU SYNTHÉTISATEUR ---
AudioSynthWaveform sine0, sine1, sine2, sine3;
AudioSynthWaveform sine4, sine5, sine6, sine7;
AudioSynthWaveform sine8, sine9, sine10, sine11;
AudioSynthWaveform sine12, sine13, sine14, sine15;
AudioSynthWaveform *waves[16] = {
  &sine0, &sine1, &sine2, &sine3, &sine4, &sine5, &sine6, &sine7,
  &sine8, &sine9, &sine10, &sine11, &sine12, &sine13, &sine14, &sine15
};

// On utilise 4 types d'ondes différentes pour bien différencier les 4 boutons
short wave_type[16] = {
  WAVEFORM_SQUARE,   // Bouton 1
  WAVEFORM_TRIANGLE, // Bouton 2
  WAVEFORM_SAWTOOTH, // Bouton 3
  WAVEFORM_SINE,     // Bouton 4
  WAVEFORM_SINE, WAVEFORM_SQUARE, WAVEFORM_SAWTOOTH, WAVEFORM_TRIANGLE,
  WAVEFORM_SINE, WAVEFORM_SQUARE, WAVEFORM_SAWTOOTH, WAVEFORM_TRIANGLE,
  WAVEFORM_SINE, WAVEFORM_SQUARE, WAVEFORM_SAWTOOTH, WAVEFORM_TRIANGLE
};

AudioEffectEnvelope env0, env1, env2, env3;
AudioEffectEnvelope env4, env5, env6, env7;
AudioEffectEnvelope env8, env9, env10, env11;
AudioEffectEnvelope env12, env13, env14, env15;
AudioEffectEnvelope *envs[16] = {
  &env0, &env1, &env2, &env3, &env4, &env5, &env6, &env7,
  &env8, &env9, &env10, &env11, &env12, &env13, &env14, &env15
};

AudioConnection patchCord01(sine0, env0);
AudioConnection patchCord02(sine1, env1);
AudioConnection patchCord03(sine2, env2);
AudioConnection patchCord04(sine3, env3);
AudioConnection patchCord05(sine4, env4);
AudioConnection patchCord06(sine5, env5);
AudioConnection patchCord07(sine6, env6);
AudioConnection patchCord08(sine7, env7);
AudioConnection patchCord09(sine8, env8);
AudioConnection patchCord10(sine9, env9);
AudioConnection patchCord11(sine10, env10);
AudioConnection patchCord12(sine11, env11);
AudioConnection patchCord13(sine12, env12);
AudioConnection patchCord14(sine13, env13);
AudioConnection patchCord15(sine14, env14);
AudioConnection patchCord16(sine15, env15);

AudioMixer4     mixer1;
AudioMixer4     mixer2;
AudioMixer4     mixer3;
AudioMixer4     mixer4;

AudioConnection patchCord17(env0, 0, mixer1, 0);
AudioConnection patchCord18(env1, 0, mixer1, 1);
AudioConnection patchCord19(env2, 0, mixer1, 2);
AudioConnection patchCord20(env3, 0, mixer1, 3);
AudioConnection patchCord21(env4, 0, mixer2, 0);
AudioConnection patchCord22(env5, 0, mixer2, 1);
AudioConnection patchCord23(env6, 0, mixer2, 2);
AudioConnection patchCord24(env7, 0, mixer2, 3);
AudioConnection patchCord25(env8, 0, mixer3, 0);
AudioConnection patchCord26(env9, 0, mixer3, 1);
AudioConnection patchCord27(env10, 0, mixer3, 2);
AudioConnection patchCord28(env11, 0, mixer3, 3);
AudioConnection patchCord29(env12, 0, mixer4, 0);
AudioConnection patchCord30(env13, 0, mixer4, 1);
AudioConnection patchCord31(env14, 0, mixer4, 2);
AudioConnection patchCord32(env15, 0, mixer4, 3);

AudioMixer4     mixerLeft;
AudioMixer4     mixerRight;
AudioOutputI2S  audioOut;

AudioConnection patchCord33(mixer1, 0, mixerLeft, 0);
AudioConnection patchCord34(mixer2, 0, mixerLeft, 1);
AudioConnection patchCord35(mixer3, 0, mixerLeft, 2);
AudioConnection patchCord36(mixer4, 0, mixerLeft, 3);
AudioConnection patchCord37(mixer1, 0, mixerRight, 0);
AudioConnection patchCord38(mixer2, 0, mixerRight, 1);
AudioConnection patchCord39(mixer3, 0, mixerRight, 2);
AudioConnection patchCord40(mixer4, 0, mixerRight, 3);
AudioConnection patchCord41(mixerLeft, 0, audioOut, 0);
AudioConnection patchCord42(mixerRight, 0, audioOut, 1);

AudioControlSGTL5000 codec;

// --- CONFIGURATION DES 4 BOUTONS ---
Bounce bouton1 = Bounce(); 
Bounce bouton2 = Bounce(); 
Bounce bouton3 = Bounce(); 
Bounce bouton4 = Bounce(); 

// Définition des broches
const int pinBouton1 = 24; 
const int pinBouton2 = 26; 
const int pinBouton3 = 28; 
const int pinBouton4 = 30; 

void setup() {
  Serial.begin(115200);
  AudioMemory(18); 
  
  codec.enable();
  codec.volume(0.45); 

  mixerLeft.gain(1, 0.36);
  mixerLeft.gain(3, 0.36);
  mixerRight.gain(0, 0.36);
  mixerRight.gain(2, 0.36);

  // Configuration du style de son (percussif)
  for (int i=0; i<16; i++) {
    envs[i]->attack(10.0);   
    envs[i]->hold(10.0);     
    envs[i]->decay(150.0);   
    envs[i]->sustain(0.0);   
    envs[i]->release(50.0);  
  }

  // --- INITIALISATION MATÉRIELLE DES 4 BOUTONS ---
  pinMode(pinBouton1, INPUT_PULLUP);
  bouton1.attach(pinBouton1);
  bouton1.interval(10); 

  pinMode(pinBouton2, INPUT_PULLUP);
  bouton2.attach(pinBouton2);
  bouton2.interval(10); 

  pinMode(pinBouton3, INPUT_PULLUP);
  bouton3.attach(pinBouton3);
  bouton3.interval(10); 

  pinMode(pinBouton4, INPUT_PULLUP);
  bouton4.attach(pinBouton4);
  bouton4.interval(10); 
  
  Serial.println("Synthetiseur 4 boutons pret !");
}

void loop() {
  // 1. Mise à jour de l'état des 4 boutons
  bouton1.update();
  bouton2.update();
  bouton3.update();
  bouton4.update();

  // 2. Action si on appuie sur le BOUTON 1
  if (bouton1.fell()) {
    Serial.println("Bouton 1 -> Note Grave");
    AudioNoInterrupts(); 
    waves[0]->begin(AMPLITUDE, 110.0, wave_type[0]); // 110 Hz = Note grave
    envs[0]->noteOn();   
    AudioInterrupts();   
  }

  // 3. Action si on appuie sur le BOUTON 2
  if (bouton2.fell()) {
    Serial.println("Bouton 2 -> Note Moyenne");
    AudioNoInterrupts(); 
    waves[1]->begin(AMPLITUDE, 220.0, wave_type[1]); // 220 Hz 
    envs[1]->noteOn();   
    AudioInterrupts();   
  }

  // 4. Action si on appuie sur le BOUTON 3
  if (bouton3.fell()) {
    Serial.println("Bouton 3 -> Note Haute");
    AudioNoInterrupts(); 
    waves[2]->begin(AMPLITUDE, 440.0, wave_type[2]); // 440 Hz 
    envs[2]->noteOn();   
    AudioInterrupts();   
  }

  // 5. Action si on appuie sur le BOUTON 4
  if (bouton4.fell()) {
    Serial.println("Bouton 4 -> Note Tres Haute");
    AudioNoInterrupts(); 
    waves[3]->begin(AMPLITUDE, 880.0, wave_type[3]); // 880 Hz 
    envs[3]->noteOn();   
    AudioInterrupts();   
  }
}