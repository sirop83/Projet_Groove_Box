# 🎛️ WHOOP BOX (Projet Groove Box)

La **Whoop Box** est un instrument de musique électronique autonome (Groove Box) basé sur un microcontrôleur Teensy. Elle permet de mixer des boucles audio en temps réel, de naviguer à travers plusieurs styles musicaux et d'enregistrer ses propres samples vocaux directement sur une carte SD.

Projet réalisé dans le cadre du cycle préparatoire intégré (CIN2) à l'ISEN Méditerranée.

---

## ✨ Fonctionnalités Principales

* **🎧 Mode Live (Séquenceur temporel) :** Lecture simultanée de 4 pistes audio (.WAV) synchronisées sur un BPM défini.
* **🎹 Styles d'Usine :** 7 kits intégrés (Hip-Hop, Electro, Lo-Fi, Chill, Orchestre, Horreur, Mege) avec des variations de BPM et de signatures rythmiques.
* **🎤 Studio Micro (Packs Personnalisés) :** Enregistrement audio en temps réel via un microphone I2S, sauvegarde directe sur carte SD et création de banques de sons (jusqu'à 8 boutons par pack).
* **🎚️ Contrôle DSP en Temps Réel :** Ajustement du volume individuel/global et application d'un filtre (Passe-Bas) sur chaque piste via des potentiomètres.
* **📺 Interface Utilisateur OLED :** Navigation par menus dynamiques, retour visuel des pistes actives (avatars animés) et barre de progression rythmique.

---

## 🛠️ Matériel (Hardware)

* **Microcontrôleur :** Carte Teensy (compatible avec la *Teensy Audio Library*)
* **Écran :** OLED SSD1306 (128x64) via I2C
* **Contrôles Physiques :** * 8x Boutons d'arcade (déclenchement des pistes/samples)
    * 1x Encodeur rotatif avec bouton poussoir (navigation menus/sélection)
    * 2x Potentiomètres analogiques (Volume et Filtre DSP)
* **Audio :** Module Audio SGTL5000 (DAC/ADC), lecteur de carte SD intégré (SPI), Microphone.

---

## 🏗️ Architecture Logicielle (Software)

Le code est modulaire, découpé en plusieurs fichiers pour séparer la logique matérielle, l'affichage et le traitement audio :

* `Groove_Box.ino` : Fichier principal abritant la machine d'états (State Machine) et la boucle principale (30 FPS).
* `Config.h` : Centralisation du routage des pins et des constantes système.
* `Controls.ino` : Détection des événements (librairie *Bounce2*), gestion de l'encodeur et logique de navigation.
* `AudioEngine.ino` : Gestion du flux audio non-bloquant (lecture SD, mixage, routage des filtres, capture micro) via la *Teensy Audio Library*.
* `Display_UI.ino` : Moteur de rendu graphique (librairie *U8g2*), affichage des menus et des animations en fonction de l'état de la machine.
* `Bitmaps.h` : Stockage des tableaux d'octets pour les sprites et éléments graphiques (avatars, QR Code).

---

## 🚀 Installation & Déploiement

### Prérequis
1. **Arduino IDE** avec l'extension **Teensyduino** installée.
2. Bibliothèques requises :
   * `U8g2` (pour l'écran OLED)
   * `Encoder` (pour la molette rotative)
   * `Bounce2` (pour l'anti-rebond des boutons)
   * *Teensy Audio Library* (généralement incluse avec Teensyduino)

### Préparation de la Carte SD
La carte SD doit être formatée en **FAT32** ou **exFAT**.
Les fichiers audio de base doivent se trouver à la racine, nommés selon la nomenclature : `[NumeroKit]_BASS.WAV`, `[NumeroKit]_LEAD.WAV`, etc. (Format : 44.1kHz, 16-bit, PCM).

### Téléversement
1. Ouvrez `Groove_Box.ino` dans l'IDE Arduino.
2. Sélectionnez le bon modèle de carte Teensy dans `Outils > Type de carte`.
3. Compilez et téléversez !

---

## 👥 Équipe & Auteurs

* **Théo Romond-Dupuis** - *Développement logiciel & Architecture logicielle*
* *(Ajoute les noms de tes coéquipiers ici (Lou, Mathys, Alla...) avec leurs rôles respectifs : Design de la boîte, Électronique, Site Web, etc.)*