#include "Config.h"
#include "Bitmaps.h"

// Affiche l'écran de démarrage avec le logo et la barre de chargement
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

// Dessine le menu de sélection des kits d'usine avec son système de défilement
void drawMenuScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(42, 12, "STYLES :");
  
  const char* nomDesKits[] = {"Hip-Hop (4)", "Electro (4)", "Lo-Fi (4)", "Chill (4)", "Orchestre (8)", "Horreur (8)", "Mege (8)"};
  int nombreTotalDeKits = 7;

  static int indexDepart = 0; 
  int cursorIndex = currentKit - 1; 

  if (cursorIndex < indexDepart) {
    indexDepart = cursorIndex;
  }
  else if (cursorIndex >= indexDepart + 3) {
    indexDepart = cursorIndex - 2;
  }

  for (int i = 0; i < 3; i++) {
    if (indexDepart + i >= nombreTotalDeKits) break;
    int indexDuKit = indexDepart + i;
    int yPos = 30 + (i * 15);
    if (currentKit == indexDuKit + 1) {
      u8g2.drawStr(10, yPos, ">");
    }
    
    u8g2.drawStr(22, yPos, nomDesKits[indexDuKit]);
  }

  if (nombreTotalDeKits > 3) {
    int trackX = 122;
    int trackY = 22;  
    int trackW = 4;   
    int trackH = 40;  

    u8g2.drawFrame(trackX, trackY, trackW, trackH);
    int maxIndexDepart = nombreTotalDeKits - 3;
    int hauteurCurseur = (3 * trackH) / nombreTotalDeKits; 
    int espaceLibre = trackH - hauteurCurseur;
    
    int scrollY = trackY;
    if (maxIndexDepart > 0) {
      scrollY += (indexDepart * espaceLibre) / maxIndexDepart;
    }

    u8g2.drawBox(trackX, scrollY, trackW, hauteurCurseur);
  }
}

// Génère l'interface de performance avec l'état des pistes, les avatars et la progression de la boucle
void drawLiveScreen() {
  for (int i = 0; i < 4; i++) {
    int startX = i * 32;
    int miniBarWidth = trackVolumes[i] * 24; 
    if (miniBarWidth > 24) miniBarWidth = 24;
    if (miniBarWidth < 0) miniBarWidth = 0;
    u8g2.drawFrame(startX + 4, 8, 24, 5);    
    u8g2.drawBox(startX + 4, 8, miniBarWidth, 5);

    u8g2.setFont(u8g2_font_4x6_tf);
    char cursorBuf[20];

    if (i == selectedTrackIdx) {
      if (trackActive[i]) {
        if (liveMode == SELECT_TRACK) {
          sprintf(cursorBuf, "B%d v", trackSound[i] + 1);
          u8g2.drawStr(startX + 8, 6, cursorBuf);
        } else if (liveMode == ADJUST_TRACK_VOLUME) {
          sprintf(cursorBuf, "B%d *v*", trackSound[i] + 1);
          u8g2.drawStr(startX + 4, 6, cursorBuf);
        }
      } else {
        if (liveMode == SELECT_TRACK) {
          u8g2.drawStr(startX + 14, 6, "v");
        } else if (liveMode == ADJUST_TRACK_VOLUME) {
          u8g2.drawStr(startX + 10, 6, "*v*");
        }
      }
    } else {
      if (trackActive[i]) {
        sprintf(cursorBuf, "B%d", trackSound[i] + 1);
        u8g2.drawStr(startX + 12, 6, cursorBuf);
      }
    }

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

// Affiche la liste des catégories principales de la Groove Box
void drawMainMenu() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(12, 12, "MENU PRINCIPAL");

  const char* optionsMenu[] = {"Styles", "Micro", "Info"}; 
  int nombreTotalOptions = 3;
  static int indexDepartMain = 0;
  int cursorIndex = mainMenuSelection; 

  if (cursorIndex < indexDepartMain) {
    indexDepartMain = cursorIndex;
  } else if (cursorIndex >= indexDepartMain + 3) {
    indexDepartMain = cursorIndex - 2;
  }

  for (int i = 0; i < 3; i++) {
    if (indexDepartMain + i >= nombreTotalOptions) break;
    int indexOption = indexDepartMain + i;
    int yPos = 30 + (i * 15);
    if (mainMenuSelection == indexOption) {
      u8g2.drawStr(10, yPos, ">");
    }
    
    u8g2.drawStr(22, yPos, optionsMenu[indexOption]);
  }

  if (nombreTotalOptions > 3) {
    int trackX = 122; int trackY = 22;
    int trackW = 4; int trackH = 40;
    u8g2.drawFrame(trackX, trackY, trackW, trackH);
    
    int maxIndexDepart = nombreTotalOptions - 3;
    int hauteurCurseur = (3 * trackH) / nombreTotalOptions; 
    int espaceLibre = trackH - hauteurCurseur;
    
    int scrollY = trackY;
    if (maxIndexDepart > 0) {
      scrollY += (indexDepartMain * espaceLibre) / maxIndexDepart;
    }
    u8g2.drawBox(trackX, scrollY, trackW, hauteurCurseur);
  }
}

// Affiche le code QR pour rediriger vers la documentation du projet
void drawInfoScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 20, "Scannez");
  u8g2.drawStr(5, 35, "pour lire");
  u8g2.drawStr(5, 50, "le manuel");

  u8g2.drawBitmap(78, 8, 6, 48, qr_nouveau);
}

// Trace une animation circulaire indiquant le temps de maintien nécessaire sur l'encodeur
void drawLongPressPopup() {
  int cx = 64; 
  int cy = 32; 
  int r = 14;  

  u8g2.setDrawColor(0);      
  u8g2.drawDisc(cx, cy, r);  
  u8g2.setDrawColor(1);
  u8g2.drawCircle(cx, cy, r);

  int angleMax = map(longPressProgress, 0, 100, 0, 360);
  for (int a = 0; a <= angleMax; a += 1) { 
    float radian = a * 3.14159 / 180.0;
    int endX = cx + (r * sin(radian));
    int endY = cy - (r * cos(radian)); 
    
    u8g2.drawLine(cx, cy, endX, endY);
  }
}

// Affiche le menu de gestion et de création des packs vocaux personnalisés
void drawMicScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(12, 12, "STUDIO MICRO");

  int activeCount = 0;
  int activePackIndices[MAX_MIC_TRACKS];
  for (int i = 0; i < MAX_MIC_TRACKS; i++) {
    if (packExists[i]) {
      activePackIndices[activeCount] = i;
      activeCount++;
    }
  }

  int totalItems = activeCount + 1; 
  
  static int indexDepartMic = 0;
  int cursorIndex = micMenuSelection; 

  if (cursorIndex < indexDepartMic) {
    indexDepartMic = cursorIndex;
  } else if (cursorIndex >= indexDepartMic + 3) {
    indexDepartMic = cursorIndex - 2;
  }

  for (int i = 0; i < 3; i++) {
    int indexOption = indexDepartMic + i;
    if (indexOption >= totalItems) break;

    int yPos = 30 + (i * 15);
    
    if (micMenuSelection == indexOption) {
      u8g2.drawStr(10, yPos, ">");
    }
    
    if (indexOption < activeCount) {
      char buf[20];
      sprintf(buf, "Pack %d", activePackIndices[indexOption] + 1); 
      u8g2.drawStr(22, yPos, buf);
    } 
    else {
      u8g2.drawStr(22, yPos, "+ Ajouter pack");
    }
  }

  if (totalItems > 3) {
    int trackX = 122; int trackY = 22;
    int trackW = 4; int trackH = 40;
    u8g2.drawFrame(trackX, trackY, trackW, trackH);
    
    int maxIndexDepart = totalItems - 3;
    int hauteurCurseur = (3 * trackH) / totalItems; 
    int espaceLibre = trackH - hauteurCurseur;
    
    int scrollY = trackY;
    if (maxIndexDepart > 0) {
      scrollY += (indexDepartMic * espaceLibre) / maxIndexDepart;
    }
    u8g2.drawBox(trackX, scrollY, trackW, hauteurCurseur);
  }
}

// Propose les actions contextuelles liées à un pack micro spécifique
void drawMicPackScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  char headerBuf[20];
  sprintf(headerBuf, "PACK %d", selectedMicPackIdx + 1);
  u8g2.drawStr(12, 12, headerBuf);

  const char* packOptions[] = {"Modifier", "Supprimer", "Lecture"};
  
  for (int i = 0; i < 3; i++) {
    int yPos = 30 + (i * 15);
    
    if (micPackMenuSelection == i) {
      u8g2.drawStr(10, yPos, ">");
    }
    
    u8g2.drawStr(22, yPos, packOptions[i]);
  }
}

// Demande une validation explicite avant la destruction d'un pack audio existant
void drawMicDeleteConfirmScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  char alertBuf[32];
  sprintf(alertBuf, "Supprimer PACK %d ?", selectedMicPackIdx + 1);
  u8g2.drawStr(5, 20, alertBuf);

  if (micDeleteConfirmSelection == 0) u8g2.drawStr(30, 40, ">");
  u8g2.drawStr(42, 40, "Non");

  if (micDeleteConfirmSelection == 1) u8g2.drawStr(30, 55, ">");
  u8g2.drawStr(42, 55, "Oui");
}

// Invite l'utilisateur à associer son futur enregistrement à un bouton d'arcade
void drawMicRecordReadyScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(3, 25, "Choisissez un bouton");
  u8g2.drawStr(12, 45, "(B1 a B8) pour REC"); 
}

// Affiche la jauge de remplissage correspondant à la durée maximale d'une prise micro
void drawMicRecordingScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  char recTitle[25];
  sprintf(recTitle, "REC : BOUTON %d", chosenRecordBtn + 1);
  u8g2.drawStr(20, 15, recTitle); 
  
  u8g2.drawStr(10, 32, "ENREGISTREMENT...");

  u8g2.drawFrame(14, 45, 100, 10);
  unsigned long elapsed = millis() - recordTimer;
  if (elapsed > loopLengthMs) elapsed = loopLengthMs;
  
  int progWidth = (elapsed * 100) / loopLengthMs;
  u8g2.drawBox(14, 45, progWidth, 10);
}

// Indique le succès de la sauvegarde du fichier vocal sur la carte SD
void drawMicRecordDoneScreen() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(11, 25, "Enregistrement OK !");
  
  char successBuf[30];
  sprintf(successBuf, "Save sur bouton %d", chosenRecordBtn + 1);
  u8g2.drawStr(16, 45, successBuf);
}