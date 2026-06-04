#include "Config.h"
#include "Bitmaps.h"

void drawBootScreen() {
  u8g2.clearBuffer(); 
  
  // CORRECTION : Police compacte de 9x15 pixels (garantit que ça ne dépasse pas)
  u8g2.setFont(u8g2_font_9x15_tf);
  // Centrage parfait : (128 de large - 90 de texte) / 2 = 19
  u8g2.drawStr(19, 30, "GROOVE BOX");
  
  // Temps de chargement à 1500 ms (1.5 seconde)
  int progress = ((millis() - bootTimer) * 100) / 1500; 
  if (progress > 100) progress = 100;
  
  u8g2.drawFrame(14, 45, 100, 10);
  u8g2.drawBox(14, 45, progress, 10);
  
  u8g2.sendBuffer(); 
}
void drawMenuScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(10, 12, "STYLE :");

  // --- 1. TA LISTE DE KITS COMPLÈTE ---
  const char* nomDesKits[] = {"Hip-Hop", "Electro", "Lo-Fi", "Chill"};
  int nombreTotalDeKits = 4;

  // --- 2. LA CAMÉRA "INTELLIGENTE" (À BUTÉE) ---
  static int indexDepart = 0; // "static" mémorise la position entre chaque image
  int cursorIndex = currentKit - 1; // La vraie position de ton encodeur (0 à 3)

  // Si le curseur monte au-dessus de la zone visible, la caméra remonte
  if (cursorIndex < indexDepart) {
    indexDepart = cursorIndex;
  }
  // Si le curseur descend en dessous de la zone visible (les 3 lignes), la caméra descend
  else if (cursorIndex >= indexDepart + 3) {
    indexDepart = cursorIndex - 2;
  }

  // --- 3. LE DESSIN DES 3 LIGNES VISIBLES ---
  for (int i = 0; i < 3; i++) {
    if (indexDepart + i >= nombreTotalDeKits) break; 

    int indexDuKit = indexDepart + i;
    int yPos = 30 + (i * 15); 

    if (currentKit == indexDuKit + 1) {
      u8g2.drawStr(10, yPos, ">"); 
    }
    
    u8g2.drawStr(22, yPos, nomDesKits[indexDuKit]);
  }

  // --- 4. LA BARRE DE SCROLL DROITE ---
  if (nombreTotalDeKits > 3) {
    int trackX = 122; 
    int trackY = 22;  
    int trackW = 4;   
    int trackH = 40;  

    u8g2.drawFrame(trackX, trackY, trackW, trackH);

    int maxIndexDepart = nombreTotalDeKits - 3;
    int hauteurCurseur = (3 * trackH) / nombreTotalDeKits; 
    int espaceLibre = trackH - hauteurCurseur;
    
    // On calcule la position visuelle du curseur de scroll
    int scrollY = trackY;
    if (maxIndexDepart > 0) {
      scrollY += (indexDepart * espaceLibre) / maxIndexDepart;
    }

    u8g2.drawBox(trackX, scrollY, trackW, hauteurCurseur);
  }
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

    // --- AFFICHAGE DES AVATARS STATIQUES ---
    int largeurImage = 24; 
    int hauteurImage = 40; 
    int yImage = 16; 
    
    if (i == 0) { 
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, mathys_onNewPiskel_2);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, mathys_offNewPiskel_3);
    }
    else if (i == 1) { 
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Alla_onNewPiskel_4);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Alla_offNewPiskel_5);
    }
    else if (i == 2) { 
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Theo_onNewPiskel_6);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Theo_offNewPiskel_7);
    }
    else if (i == 3) { 
      if (trackActive[i]) u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Lou_onNewPiskel_8);
      else                u8g2.drawXBMP(startX + 4, yImage, largeurImage, hauteurImage, Lou_offNewPiskel_9);
    }
  }

  // --- 2. BARRE DE PROGRESSION DE LA BOUCLE PERMANENTE ---
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