#include "Config.h"
#include "Bitmaps.h" // <-- ON IMPORTE VOS PERSONNAGES ICI !

void drawBootScreen() {
  u8g2.clearBuffer(); 
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(12, 30, "GROOVE BOX");
  
  int progress = ((millis() - bootTimer) * 100) / 3000; 
  if (progress > 100) progress = 100;
  
  u8g2.drawFrame(14, 45, 100, 10);
  u8g2.drawBox(14, 45, progress, 10);
  u8g2.sendBuffer(); 
}

void drawMenuScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(10, 12, "STYLE :");

  if (currentKit == 1) u8g2.drawStr(10, 30, "> Hip-Hop");
  else                 u8g2.drawStr(10, 30, "  Hip-Hop");

  if (currentKit == 2) u8g2.drawStr(10, 45, "> Synthwave");
  else                 u8g2.drawStr(10, 45, "  Synthwave");

  if (currentKit == 3) u8g2.drawStr(10, 60, "> Lo-Fi");
  else                 u8g2.drawStr(10, 60, "  Lo-Fi");
}

void drawLiveScreen() {
  // --- 1. DESSIN DES 4 PISTES ---
  for (int i = 0; i < 4; i++) {
    int startX = i * 32;
    
    // Les mini-jauges individuelles
    int miniBarWidth = trackVolumes[i] * 24; 
    if (miniBarWidth > 24) miniBarWidth = 24;
    if (miniBarWidth < 0) miniBarWidth = 0;
    
    u8g2.drawFrame(startX + 4, 8, 24, 5);    
    u8g2.drawBox(startX + 4, 8, miniBarWidth, 5); 
    
    // Le curseur de sélection
    if (i == selectedTrackIdx) {
      u8g2.setFont(u8g2_font_4x6_tf); 
      if (liveMode == SELECT_TRACK) {
        u8g2.drawStr(startX + 14, 6, "v");   
      } else if (liveMode == ADJUST_TRACK_VOLUME) {
        u8g2.drawStr(startX + 10, 6, "*v*"); 
      }
    }

    // --- LE REMPLACEMENT : AFFICHAGE DES AVATARS ---
    
    // Renseigne ici la vraie taille de votre dessin exporté depuis Piskel
    int largeurImage = 24; 
    int hauteurImage = 40; 
    
    // Ajuste yImage si le personnage flotte ou est trop bas
    int yImage = 16; 
    
    // XBMP lit le tableau dans la mémoire PROGMEM
    if (i == 0) { // Piste 1 (BASS) : Mathys
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, mathys_onNewPiskel_2);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, mathys_offNewPiskel_3);
    }
    else if (i == 1) { // Piste 2 (LEAD) : Alla
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Alla_onNewPiskel_4);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Alla_offNewPiskel_5);
    }
    else if (i == 2) { // Piste 3 (PERC) : Theo
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Theo_onNewPiskel_6);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Theo_offNewPiskel_7);
    }
    else if (i == 3) { // Piste 4 (BEAT) : Lou
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Lou_onNewPiskel_8);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Lou_offNewPiskel_9);
    }
  }

  // --- 2. BARRE DE PROGRESSION DE LA BOUCLE ---
  u8g2.drawFrame(0, 60, 128, 4); 

  if (isRunning) {
    unsigned long currentLoopStart = nextLoopTime - loopLengthMs;
    unsigned long timeInLoop = millis() - currentLoopStart; 
    
    if (timeInLoop > loopLengthMs) {
      timeInLoop = loopLengthMs;
    }
    
    int progBarWidth = map(timeInLoop, 0, loopLengthMs, 0, 128);
    u8g2.drawBox(0, 60, progBarWidth, 4); 
  }
}