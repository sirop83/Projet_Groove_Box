#include "Config.h"

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
    
    // --- LE RETOUR DES MINI-JAUGES INDIVIDUELLES ---
    int miniBarWidth = trackVolumes[i] * 24; // Mappe de 0.0-1.0 vers 0-24 pixels
    if (miniBarWidth > 24) miniBarWidth = 24;
    if (miniBarWidth < 0) miniBarWidth = 0;
    
    // On dessine le contour et le remplissage de la mini-jauge
    u8g2.drawFrame(startX + 4, 8, 24, 5);    
    u8g2.drawBox(startX + 4, 8, miniBarWidth, 5); 
    
    // --- LE CURSEUR DE SÉLECTION ---
    if (i == selectedTrackIdx) {
      u8g2.setFont(u8g2_font_4x6_tf); 
      if (liveMode == SELECT_TRACK) {
        u8g2.drawStr(startX + 14, 6, "v");   // Simple flèche en navigation
      } else if (liveMode == ADJUST_TRACK_VOLUME) {
        u8g2.drawStr(startX + 10, 6, "*v*"); // Flèche encadrée si verrouillé
      }
    }

    // --- LE RECTANGLE PRINCIPAL DE LA PISTE (Mute / Unmute) ---
    // Statique : 40 pixels de haut. Plein si actif, vide si muet.
    if (trackActive[i]) {
      u8g2.drawBox(startX + 4, 18, 24, 40);   
    } else {
      u8g2.drawFrame(startX + 4, 18, 24, 40); 
    }
  }

  // --- 2. BARRE DE PROGRESSION DE LA BOUCLE PERMANENTE ---
  u8g2.drawFrame(0, 60, 128, 4); // Le cadre extérieur toujours visible

  if (isRunning) {
    unsigned long currentLoopStart = nextLoopTime - loopLengthMs;
    unsigned long timeInLoop = millis() - currentLoopStart; 
    
    if (timeInLoop > loopLengthMs) {
      timeInLoop = loopLengthMs;
    }
    
    int progBarWidth = map(timeInLoop, 0, loopLengthMs, 0, 128);
    u8g2.drawBox(0, 60, progBarWidth, 4); // Le remplissage rythmique
  }
}