#include "Config.h"
#include "Bitmaps.h"

void drawBootScreen() {
  u8g2.clearBuffer(); 

  u8g2.setFont(u8g2_font_9x15_tf);
  u8g2.drawStr(19, 30, "GROOVE BOX");
  
  int progress = ((millis() - bootTimer) * 100) / 1500; 
  if (progress > 100) progress = 100;
  
  u8g2.drawFrame(14, 45, 100, 10);
  u8g2.drawBox(14, 45, progress, 10);
  
  u8g2.sendBuffer(); 
}
void drawMenuScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(42, 12, "STYLES :");

  // --- 1. LISTE DE KITS COMPLÈTE ---
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

void drawMainMenu() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(12, 12, "MENU PRINCIPAL");

  u8g2.setFont(u8g2_font_ncenB08_tr);
  if (mainMenuSelection == 0) u8g2.drawStr(10, 40, "> STYLES ");
  else u8g2.drawStr(20, 40, "STYLES ");

  if (mainMenuSelection == 1) u8g2.drawStr(10, 55, "> INFO ");
  else u8g2.drawStr(20, 55, "INFO ");
}

void drawInfoScreen() {
  // 1. Le texte à gauche
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 20, "Scannez");
  u8g2.drawStr(5, 35, "pour lire");
  u8g2.drawStr(5, 50, "le manuel");

  u8g2.drawBitmap(78, 8, 6, 48, qr_nouveau);
}
void drawLongPressPopup() {
  int cx = 64; // Centre de l'écran (X)
  int cy = 32; // Centre de l'écran (Y)
  int r = 14;  // Rayon du cercle

  u8g2.setDrawColor(0);      
  u8g2.drawDisc(cx, cy, r);  
  u8g2.setDrawColor(1);      

  // 2. Le contour du cercle (en blanc)
  u8g2.drawCircle(cx, cy, r);

  int angleMax = map(longPressProgress, 0, 100, 0, 360);
  
  for (int a = 0; a <= angleMax; a += 1) { 
    float radian = a * 3.14159 / 180.0;
    int endX = cx + (r * sin(radian));
    int endY = cy - (r * cos(radian)); 
    
    u8g2.drawLine(cx, cy, endX, endY);
  }
}