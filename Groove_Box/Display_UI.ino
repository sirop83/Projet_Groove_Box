#include "Config.h"

void drawBootScreen() {
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(5, 30, "GROOVE BOX");
  
  // Petite barre de chargement animée par le temps
  int loadWidth = map(millis(), 0, 2000, 0, 100);
  if (loadWidth > 100) loadWidth = 100;
  u8g2.drawFrame(14, 45, 100, 10);
  u8g2.drawBox(14, 45, loadWidth, 10);
  
  if (millis() > 2500) currentState = STATE_MENU;
}

void drawMenuScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  // Titre plus court
  u8g2.drawStr(10, 12, "STYLE :");

  // Textes raccourcis pour respirer sur 128 pixels de large
  if (currentKit == 1) u8g2.drawStr(10, 30, "> Hip-Hop");
  else                 u8g2.drawStr(10, 30, "  Hip-Hop");

  if (currentKit == 2) u8g2.drawStr(10, 45, "> Synthwave");
  else                 u8g2.drawStr(10, 45, "  Synthwave");

  if (currentKit == 3) u8g2.drawStr(10, 60, "> Lo-Fi");
  else                 u8g2.drawStr(10, 60, "  Lo-Fi");
}

void drawLiveScreen() {
  // 1. DESSIN DES 4 COLONNES (Incredibox style)
  for (int i = 0; i < 4; i++) {
    int startX = i * 32;
    
    // Si la piste est sélectionnée par l'encodeur, on met un petit cadre au-dessus
    if (i == selectedTrackIdx) {
      if (liveMode == SELECT_TRACK) u8g2.drawFrame(startX + 4, 0, 24, 6);
      if (liveMode == ADJUST_TRACK_VOLUME) u8g2.drawBox(startX + 4, 0, 24, 6); // Plein si on règle le volume
    }

    // Le "Personnage" (Pour l'instant un rectangle, tu mettras un Bitmap ici)
    if (trackActive[i]) {
      u8g2.drawBox(startX + 4, 15, 24, 40); // Plein = Unmute
    } else {
      u8g2.drawFrame(startX + 4, 15, 24, 40); // Vide (Contours) = Mute
    }
  }

  // 2. LA LIGNE DE TEMPO QUI BALAYE
  // Pour le test sans audio, on fait avancer le curseur virtuellement
  unsigned long timeInLoop = millis() % loopLengthMs; 
  int lineX = map(timeInLoop, 0, loopLengthMs, 0, 128);
  u8g2.drawLine(lineX, 0, lineX, 64);

  // 3. LE POP-UP DU FILTRE DSP (S'affiche par-dessus si on tourne le potard)
  if (showDspPopUp) {
    u8g2.setDrawColor(0); // On dessine en noir pour effacer le fond
    u8g2.drawBox(24, 15, 80, 34);
    u8g2.setDrawColor(1); // On repasse en blanc
    u8g2.drawFrame(24, 15, 80, 34);
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(30, 28, "FILTRE DSP");
    
    // Jauge de 0 à 100%
    int gaugeWidth = map(dspValue, 0, 1023, 0, 68);
    u8g2.drawFrame(30, 35, 68, 8);
    u8g2.drawBox(30, 35, gaugeWidth, 8);
  }
}