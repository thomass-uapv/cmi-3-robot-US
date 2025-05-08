#include <AccelStepper.h> // On importe la bibliothèque AccelStepper pour contrôler les moteurs
#include <SoftwareSerial.h>

// Déclaration des pins utilisés pour les deux moteurs et les deux LED
#define STEP_PIN_1 6
#define DIR_PIN_1 7
#define ENABLE_PIN_1 8

#define STEP_PIN_2 5
#define DIR_PIN_2 4
#define ENABLE_PIN_2 9

#define LED_PIN_M 12
#define LED_PIN_SA 13


// Déclaration du module Bluetooth (RX sur broche 2, TX sur broche 3) et pour le tranfaire de données
SoftwareSerial BluetoothSerial(2, 3);
SoftwareSerial comLidar (10, 11);


char direction = '0';
int motorSpeed = 10000;

const int TAB_SIZE = 8;
int tab[TAB_SIZE];

// Indices utilisés pour identifier les directions dans le tableau des distances (radar/LiDAR)
int avant[3] {7, 0, 1};
int droit[3] {1, 2, 3};
int gauche[3] {3, 4, 5};
int arriere[3] {5, 6, 7};

// Valeurs seuils de distance, à adapter selon l'environnement du projet
int distance_face = 20;
int distance_diagonal = 15;
int distance_cote = 10;
int val_medium = 10

// Le mode manuel est activé par défaut si le LiDAR n'est pas connecté
bool modeManuel = true;

// Déclaration des moteurs en mode DRIVER avec la bibliothèque AccelStepper
AccelStepper moteur1(AccelStepper::DRIVER, STEP_PIN_1, DIR_PIN_1);
AccelStepper moteur2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2);


void setup() {
    Serial.begin(9600);
    BluetoothSerial.begin(9600);  // Initialisation Bluetooth
    comLidar.begin(9600);         // Communication série avec le module LiDAR 

    // Configuration des pins ENABLE des moteurs
    pinMode(ENABLE_PIN_1, OUTPUT); 
    pinMode(ENABLE_PIN_2, OUTPUT);

    // Les moteurs sont désactivés par défaut
    digitalWrite(ENABLE_PIN_1, HIGH);
    digitalWrite(ENABLE_PIN_2, HIGH);

    // Paramétrage de la vitesse maximale et de l'accélération
    moteur1.setMaxSpeed(2000);
    moteur2.setMaxSpeed(2000);

    moteur1.setAcceleration(200);
    moteur2.setAcceleration(200);

    // Configuration des LEDs qui indiquent le mode de conduite
    pinMode(LED_PIN_M, OUTPUT);
    pinMode(LED_PIN_SA, OUTPUT); 
}

void getDatas(int tab[], int size){
  // On attend au moins un caractère
  if (comLidar.available()) {
    // Lire size entiers (parseInt ignore automatiquement les virgules)
    for (int i = 0; i < size; i++) {
      // .parseInt(), récupère les éléments numériques
      tab[i] = comLidar.parseInt();
    }
    comLidar.read(); // consommer le '\n' final
  }
}

// Fonction de déplacement vers l'avant
void avancer() {

  // Toutes les fonctions de déplacement fonctionnent sur le même principe :
  // Elles utilisent différentes valeurs de distance selon la position des capteurs
  // et ajustent la vitesse ou le mouvement du moteur en conséquence.

  // Si on est en mode semi-automatique (et non manuel)
  if(!modeManuel){ 
    // On traite ici les 3 capteurs orientés vers l'avant (un frontal et deux en diagonale)
    for(int i = 0; i < 3; i++) {  
      // Capteur central (frontal)
      if(i == 1) {  
        // Si un obstacle est détecté à une distance moyenne (ni trop proche, ni trop loin)
        if(tab[avant[i]] < (distance_face + val_medium) && tab[avant[i]] > distance_face) { 
          digitalWrite(ENABLE_PIN_1, LOW); // Activation du moteur
          moteur1.setSpeed(motorSpeed / 2); // Réduction de la vitesse à moitié pour ralentir
        }
        // Si la voie est libre à distance normale ou grande
        else if(tab[avant[i]] > distance_face) {
          digitalWrite(ENABLE_PIN_1, LOW); 
          moteur1.setSpeed(motorSpeed); // On avance à pleine vitesse
        }
        // Si l'obstacle est trop proche : on ne fait rien, le moteur n'est pas mis à jour
        // Cela permet d'éviter une collision
      }
      else { // Pour les capteurs diagonaux gauche et droit
         // Même logique, mais avec les valeurs spécifiques aux angles
        if(tab[avant[i]] < (distance_diagonal + val_medium) && tab[avant[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_1, LOW);
          moteur1.setSpeed(motorSpeed / 2);
        }
        else if(tab[avant[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_1, LOW);
          moteur1.setSpeed(motorSpeed);
        }
      }
    }
  }
  else {// En mode manuel : on n’utilise pas les capteurs, on avance directement
    digitalWrite(ENABLE_PIN_1, LOW);
    moteur1.setSpeed(motorSpeed);
  }
}

// Fonction de déplacement en marche arrière
void reculer() {
  if(!modeManuel){
    for(int i = 0; i < 3; i++) {
      if(i == 1) {
        if(tab[arriere[i]] < (distance_face + val_medium) && tab[arriere[i]] > distance_face) {
          digitalWrite(ENABLE_PIN_1, LOW);
          moteur1.setSpeed(-motorSpeed / 2);
        }
        else if(tab[arriere[i]] > distance_face) {
          digitalWrite(ENABLE_PIN_1, LOW);
          moteur1.setSpeed(-motorSpeed);
        }
      }
      else {
        if(tab[arriere[i]] < (distance_diagonal + val_medium) && tab[arriere[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_1, LOW);
          moteur1.setSpeed(-motorSpeed / 2);
        }
        else if(tab[arriere[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_1, LOW);
          moteur1.setSpeed(-motorSpeed);
        }
      }
    }
  }
  else {
    digitalWrite(ENABLE_PIN_1, LOW);
    moteur1.setSpeed(-motorSpeed);
  }
}

// Fonction de déplacement vers la droite
void droit() {
  if(modeManuel){
    for(int i = 0; i < 3; i++) {
      if(i == 1) {
        if(tab[droit[i]] < (distance_cote + val_medium) && tab[droit[i]] > distance_cote) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(-motorSpeed / 2);
        }
        else if(tab[droit[i]] > distance_cote) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(-motorSpeed);
        }
      }
      else {
        if(tab[droit[i]] < (distance_diagonal + val_medium) && tab[droit[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(-motorSpeed / 2);  
        }
        else if(tab[droit[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(-motorSpeed);
        }
      }
    }
  }
  else {
    digitalWrite(ENABLE_PIN_2, LOW);
    moteur2.setSpeed(-motorSpeed);
  }
}

// Fonction de déplacement vers la gauche
void gauche() {
  if(modeManuel){
    for(int i = 0; i < 3; i++) {
      if(i == 1) {
        if(tab[gauche[i]] < (distance_cote + val_medium) && tab[gauche[i]] > distance_cote) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(motorSpeed / 2);
        }
        else if(tab[gauche[i]] > distance_cote) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(motorSpeed);
        }
      }
      else {
        if(tab[gauche[i]] < (distance_diagonal + val_medium) && tab[gauche[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(motorSpeed / 2);  
        }
        else if(tab[gauche[i]] > distance_diagonal) {
          digitalWrite(ENABLE_PIN_2, LOW);
          moteur2.setSpeed(motorSpeed);
        }
      }
    }
  }
  else {
    digitalWrite(ENABLE_PIN_2, LOW);
    moteur2.setSpeed(motorSpeed);
  }
}

// Désactive le moteur 1 pour économiser la batterie
void stop1() {
  digitalWrite(ENABLE_PIN_1, HIGH);
  moteur1.setSpeed(0);
}

// Désactive le moteur 2 pour économiser la batterie
void stop2() { 
  digitalWrite(ENABLE_PIN_2, HIGH);
  moteur2.setSpeed(0);
}

// Stoppe les deux moteurs simultanément en cas d'urgence
void stop() {
  moteur1.setSpeed(0);
  digitalWrite(ENABLE_PIN_1, HIGH);
  moteur2.setSpeed(0);
  digitalWrite(ENABLE_PIN_2, HIGH);
}

void loop() {
  getDatas(tab, TAB_SIZE);  // Récupération des valeurs du LiDAR
  sendToApp(tab); // Transmission des données à l'application

  if (BluetoothSerial.available() > 0) {
      direction = BluetoothSerial.read(); // Lit un caractère envoyer par l'aplication
      
      Serial.println(direction); // Affiche la valeur reçue en ASCII

  // on compare le caractère afin de savoire quelle informations est demmanté par l'utilisateur 
      if (direction == '1') { 
            avancer();
        }
      if (direction == '2') {
            reculer();
        }
      if (direction == '3') {
           droit();
        }
      if (direction == '4') {
            gauche();
        }
      if (direction == '5') {
            stop1();
        }
      if (direction == '6') {
            stop2();
        }
      if (direction == '0') {
            stop();
        }
      if (direction == '7') {
            modeManuel == true;
        }
      if (direction == '8') {
            modeManuel == false;
        }
  }

  // on allume la LED qui indique quelle mode de conduite est tulisé 
  if(modeManuel) {
    digitalWrite(12, HIGH);
    digitalWrite(13, LOW);
  } else {
    digitalWrite(12, LOW);
    digitalWrite(13, HIGH);
  }

  // on fait trouné les moteur en fonctions des instructions envoyer 
    moteur1.runSpeed();
    moteur2.runSpeed();
}
