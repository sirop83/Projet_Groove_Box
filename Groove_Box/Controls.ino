#include "Config.h"

long oldEncPosition = 0;
unsigned long btnPressTime = 0;
bool isLongPress = false;

Bounce btn1 = Bounce();
Bounce btn2 = Bounce();
Bounce btn3 = Bounce(); 
Bounce btn4 = Bounce();
Bounce btn5 = Bounce();
Bounce btn6 = Bounce(); 
Bounce btn7 = Bounce(); 
Bounce btn8 = Bounce();

void setupControls() {
  pinMode(PIN_ENC_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BTN), encoderBtnISR, CHANGE);
  
  pinMode(PIN_B1, INPUT_PULLUP); btn1.attach(PIN_B1); btn1.interval(10);
  pinMode(PIN_B2, INPUT_PULLUP); btn2.attach(PIN_B2); btn2.interval(10);
  pinMode(PIN_B3, INPUT_PULLUP); btn3.attach(PIN_B3); btn3.interval(10);
  pinMode(PIN_B4, INPUT_PULLUP); btn4.attach(PIN_B4); btn4.interval(10);
  
  pinMode(PIN_B5, INPUT_PULLUP); btn5.attach(PIN_B5); btn5.interval(10);
  pinMode(PIN_B6, INPUT_PULLUP); btn6.attach(PIN_B6); btn6.interval(10);
  pinMode(PIN_B7, INPUT_PULLUP); btn7.attach(PIN_B7); btn7.interval(10);
  pinMode(PIN_B8, INPUT_PULLUP); btn8.attach(PIN_B8); btn8.interval(10);

  dspValue = 1023 - analogRead(PIN_POT_VOL); 
}

void resetMachine() {
  isRunning = false;
  nextLoopTime = 0;
  liveMode = SELECT_TRACK;
  selectedTrackIdx = 0;
  for (int i = 0; i < 4; i++) {
    trackActive[i] = false;
    trackVolumes[i] = 0.5; 
    trackFilters[i] = 15000.0;
    trackSound[i] = i;
  }
  stopAllAudio();
}

void piloterPiste(int i) {
  trackActive[i] = !trackActive[i];
  if (!isRunning && trackActive[i]) {
    isRunning = true;
    nextLoopTime = millis() + loopLengthMs; 
    playTrack(i);
  }
  else if (isRunning) {
    bool plusAucunePisteActive = true;
    for (int j = 0; j < 4; j++) {
      if (trackActive[j]) plusAucunePisteActive = false;
    }
    if (plusAucunePisteActive) {
      isRunning = false;
      nextLoopTime = 0;
      stopAllAudio();
    }
  }
}

void gererBoutonSon(int numSon) {
  for (int i = 0; i < 4; i++) {
    if (trackActive[i] && trackSound[i] == numSon) {
      piloterPiste(i); 
      return; // On a fini
    }
  }
  
  for (int i = 0; i < 4; i++) {
    if (!trackActive[i]) {
      // NOUVEAU : On coupe physiquement la lecture si l'ancien son était différent
      if (trackSound[i] != numSon) {
        stopTrack(i); 
      }
      trackSound[i] = numSon;
      piloterPiste(i);       
      return; 
    }
  }
}

void updateControls() {
  btn1.update(); btn2.update(); btn3.update(); btn4.update();
  btn5.update(); btn6.update(); btn7.update(); btn8.update();

  // Détection des boutons physiques pour l'enregistrement
  if (currentState == STATE_MIC_RECORD_READY) {
    int inputBtn = -1;
    if (btn1.fell()) inputBtn = 0;
    if (btn2.fell()) inputBtn = 1;
    if (btn3.fell()) inputBtn = 2;
    if (btn4.fell()) inputBtn = 3;
    if (btn5.fell()) inputBtn = 4; 
    if (btn6.fell()) inputBtn = 5; 
    if (btn7.fell()) inputBtn = 6; 
    if (btn8.fell()) inputBtn = 7;
    
    if (inputBtn != -1) {
      chosenRecordBtn = inputBtn;
      
      // Configuration temporelle stricte à 120 BPM (2 secondes la boucle)
      currentBPM = 120.0;
      loopLengthMs = (60000.0 / currentBPM) * 16;
      
      // Nom du fichier unique : ex "M_P1_B1.WAV" (Pack 1, Bouton 1)
      char recFileName[30];
      sprintf(recFileName, "M_P%d_B%d.WAV", selectedMicPackIdx + 1, chosenRecordBtn + 1);
      
      recordTimer = millis(); // Lancement du chronomètre
      startRecording(recFileName); // Démarrage mécanique
      currentState = STATE_MIC_RECORDING;
    }
  }

  if (isrHasClicked) {
    isrHasClicked = false;
    longPressProgress = 0;
    if (!isLongPress) handleShortClick();
  }

  if (isrIsPressed) {
    unsigned long holdTime = millis() - isrPressTime;
    if (holdTime > 350 && !isLongPress) {
      longPressProgress = map(holdTime, 350, 800, 0, 100);
      if (holdTime >= 800) {
        isLongPress = true;
        longPressProgress = 0;
        
        // --- LE NOUVEL AIGUILLAGE DU BOUTON RETOUR ---
        if (currentState == STATE_LIVE) { 
          resetMachine(); // On coupe le son et on remet à zéro
          
          if (isUsingMicPack) {
            // Si on jouait un Pack Micro, on retourne dans le sous-menu du pack !
            currentState = STATE_MIC_PACK; 
          } else {
            // Si on jouait un style normal, on retourne à la liste des styles
            currentState = STATE_MENU; 
          }
        }
        else if (currentState == STATE_MENU) currentState = STATE_MAIN_MENU;
        else if (currentState == STATE_INFO) currentState = STATE_MAIN_MENU;
        else if (currentState == STATE_MIC) currentState = STATE_MAIN_MENU;
        else if (currentState == STATE_MIC_PACK) currentState = STATE_MIC;
        else if (currentState == STATE_MIC_DELETE_CONFIRM) currentState = STATE_MIC_PACK;
        else if (currentState == STATE_MIC_RECORD_READY) currentState = STATE_MIC_PACK;
      }
    }
  } else {
    isLongPress = false;
    longPressProgress = 0;
  }

  long rawEnc = myEnc.read();
  int deltaEnc = 0;
  if (rawEnc >= oldEncPosition + 4) { deltaEnc = 1; oldEncPosition = rawEnc; }
  else if (rawEnc <= oldEncPosition - 4) { deltaEnc = -1; oldEncPosition = rawEnc; }

  // Navigation Menu Micro
  if (currentState == STATE_MIC && deltaEnc != 0) {
    micMenuSelection += deltaEnc;
    
    // On compte combien on a de pistes pour bloquer la molette
    int activeCount = 0;
    for (int i = 0; i < MAX_MIC_TRACKS; i++) {
      if (packExists[i]) activeCount++;
    }
    
    int maxItems = activeCount + 1; // Packs + Bouton ajouter
    if (micMenuSelection >= maxItems) micMenuSelection = 0;
    if (micMenuSelection < 0) micMenuSelection = maxItems - 1;
  }

  // Navigation Sous-Menu d'un Pack
  if (currentState == STATE_MIC_PACK && deltaEnc != 0) {
    micPackMenuSelection += deltaEnc;
    if (micPackMenuSelection > 2) micPackMenuSelection = 0; // 3 options max (0, 1, 2)
    if (micPackMenuSelection < 0) micPackMenuSelection = 2;
  }

  if (currentState == STATE_MIC_DELETE_CONFIRM && deltaEnc != 0) {
    micDeleteConfirmSelection += deltaEnc;
    if (micDeleteConfirmSelection > 1) micDeleteConfirmSelection = 0; 
    if (micDeleteConfirmSelection < 0) micDeleteConfirmSelection = 1;
  }

  if (currentState == STATE_MAIN_MENU && deltaEnc != 0) {
    mainMenuSelection += deltaEnc;
    if (mainMenuSelection > 2) mainMenuSelection = 0;
    if (mainMenuSelection < 0) mainMenuSelection = 2;
  }

  if (currentState == STATE_MENU && deltaEnc != 0) {
    currentKit += deltaEnc;
    if (currentKit > 7) currentKit = 1; 
    if (currentKit < 1) currentKit = 7; 
  }
  else if (currentState == STATE_LIVE) {
    if (btn1.fell()) gererBoutonSon(0);
    if (btn2.fell()) gererBoutonSon(1);
    if (btn3.fell()) gererBoutonSon(2);
    if (btn4.fell()) gererBoutonSon(3);
    if (btn5.fell()) gererBoutonSon(4);
    if (btn6.fell()) gererBoutonSon(5);
    if (btn7.fell()) gererBoutonSon(6);
    if (btn8.fell()) gererBoutonSon(7);

    if (deltaEnc != 0 && liveMode == SELECT_TRACK) {
      selectedTrackIdx += deltaEnc;
      if (selectedTrackIdx > 3) selectedTrackIdx = 0;
      if (selectedTrackIdx < 0) selectedTrackIdx = 3;
      dspValue = 1023 - analogRead(PIN_POT_VOL);
    }

    int pot = 1023 - analogRead(PIN_POT_VOL);
    if (abs(pot - dspValue) > 25) {
      dspValue = pot;
      if (liveMode == SELECT_TRACK) {
        float masterLevel = pot / 1023.0;
        for (int j = 0; j < 4; j++) trackVolumes[j] = masterLevel;
      } 
      else if (liveMode == ADJUST_TRACK_VOLUME) {
        trackVolumes[selectedTrackIdx] = pot / 1023.0;
      }
    }
    
    int potDsp = analogRead(PIN_POT_DSP);
    static int lastPotDsp = -1; 
    if (abs(potDsp - lastPotDsp) > 15) { 
      lastPotDsp = potDsp;
      float freq = 40.0 * pow((15000.0 / 40.0), (potDsp / 1023.0));
      if (liveMode == SELECT_TRACK) {
        for (int j = 0; j < 4; j++) { trackFilters[j] = freq; setTrackFilter(j, freq); }
      } 
      else if (liveMode == ADJUST_TRACK_VOLUME) {
        trackFilters[selectedTrackIdx] = freq;
        setTrackFilter(selectedTrackIdx, freq);
      }
    }
  }
}

void handleShortClick() {
  if (currentState == STATE_MAIN_MENU) {
    if (mainMenuSelection == 0) {
      currentState = STATE_MENU;
    }
    else if (mainMenuSelection == 1) {
      currentState = STATE_MIC;
    }
    else if (mainMenuSelection == 2) {
      currentState = STATE_INFO;
    }
  }
  else if (currentState == STATE_MENU) {
    isUsingMicPack = false; 
    
    // NOUVEAU : On crée une variable pour le nombre de temps (16 par défaut)
    int nombreDeTemps = 16; 

    if (currentKit == 1) currentBPM = 150.0; 
    else if (currentKit == 2) currentBPM = 110.0; 
    else if (currentKit == 3) currentBPM = 78.0; 
    else if (currentKit == 4) currentBPM = 90.0; 
    else if (currentKit == 5) currentBPM = 120.0; 
    else if (currentKit == 6) currentBPM = 80.0;
    else if (currentKit == 7) {
      currentBPM = 100.0; // Le BPM de ton style numéro 7 
      nombreDeTemps = 8;  // MODIFICATION : 2 mesures = 8 temps
    }
    
    // On remplace le "* 16" figé par notre nouvelle variable
    loopLengthMs = (60000.0 / currentBPM) * nombreDeTemps;
    
    currentState = STATE_LIVE;
    liveMode = SELECT_TRACK;
  }

  else if (currentState == STATE_LIVE) {
    if (liveMode == SELECT_TRACK) liveMode = ADJUST_TRACK_VOLUME;
    else liveMode = SELECT_TRACK;
    dspValue = 1023 - analogRead(PIN_POT_VOL);
  }

  else if (currentState == STATE_MIC) {
    // On refait le repérage des vrais index
    int activeCount = 0;
    int activePackIndices[MAX_MIC_TRACKS];
    for (int i = 0; i < MAX_MIC_TRACKS; i++) {
      if (packExists[i]) {
        activePackIndices[activeCount] = i;
        activeCount++;
      }
    }

    if (micMenuSelection < activeCount) {
      // Clic sur un Pack existant -> On récupère son VRAI numéro en mémoire
      selectedMicPackIdx = activePackIndices[micMenuSelection];
      micPackMenuSelection = 0; 
      currentState = STATE_MIC_PACK;
    } 
    else if (micMenuSelection == activeCount) {
      // Clic sur "+ Ajouter piste"
      // On cherche le premier trou vide (false) de 0 à 3
      for (int i = 0; i < MAX_MIC_TRACKS; i++) {
        if (!packExists[i]) {
          packExists[i] = true; // On a trouvé un emplacement, on le réserve !
          break; // On s'arrête là, pas besoin d'en créer d'autres
        }
      }
    } 
  }
  else if (currentState == STATE_MIC_PACK) {
    if (micPackMenuSelection == 0) {
      // Clic sur "Modifier" -> On bascule en mode attente du bouton à enregistrer !
      currentState = STATE_MIC_RECORD_READY;
    } 
    else if (micPackMenuSelection == 1) {
      // Clic sur "Supprimer"
      micDeleteConfirmSelection = 0;
      currentState = STATE_MIC_DELETE_CONFIRM;
    } 
    else if (micPackMenuSelection == 2) {
      // Clic sur "Piste" -> Charge ce Pack Micro pour le jouer en live !
      isUsingMicPack = true;
      activeMicPackIdx = selectedMicPackIdx;
      currentBPM = 120.0; // Vitesse universelle pour vos pistes personnalisées
      loopLengthMs = (60000.0 / currentBPM) * 16;
      currentState = STATE_LIVE;
      liveMode = SELECT_TRACK;
    }
  }
  else if (currentState == STATE_MIC_DELETE_CONFIRM) {
    if (micDeleteConfirmSelection == 1) {
      // OUI : On supprime ! 
      
      // 1. Suppression physique des 4 fichiers audio potentiels sur la carte SD
      char fileToDelete[35];
      for (int btn = 1; btn <= 8; btn++) {
        // On recrée le nom exact du fichier pour ce pack et ce bouton
        sprintf(fileToDelete, "M_P%d_B%d.WAV", selectedMicPackIdx + 1, btn);
        
        // Si le fichier existe sur la carte SD, on l'efface sans pitié !
        if (SD.exists(fileToDelete)) {
          SD.remove(fileToDelete);
        }
      }

      // 2. Libération du "slot" virtuel dans le menu
      packExists[selectedMicPackIdx] = false; 
      
      currentState = STATE_MIC;             
      micMenuSelection = 0; // Retour en haut de la liste
    } else {
      // NON : On annule et on revient au menu du pack
      currentState = STATE_MIC_PACK;
    }
  }
}