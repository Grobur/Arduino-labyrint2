/* Actieve inputs en outputs van dit labyrint:

   Inputs:
   -  5 lichtsensoren: 2 voor power-updetectie in het midden van het labyrint,
      2 voor poortdetectie aan de randen, 1 voor de game-overdetectie onder het labyrint

   -  Joystick: gebruikt om servomotoren onder het labyrint aan te sturen

   -  Drukknop: calibratie van de servomotoren

   Outputs:

   -  LCD-scherm: om power-ups en andere zaken zoals winst of verlies weer te geven aan de gebruiker

   -  2 servomotoren: labyrint kantelen in X en Y richting zodat knikker kan rollen

   -  5 LEDS: om de lichtsensoren te belichten, de knikker rolt tussen de LED en een lichtsensor in
      om op die manier een duidelijk waardeverschil van de lichtsensor te genereren

   -  2 Neopixels: om de poort ofwel met "Winst (groen)" of "Verlies (rood)" aan te duiden,
      een andere kleur zal ook gebruikt worden wanneer een powerup geactiveerd wordt,
      om de gebruiker hier niet enkel via het LCD-scherm op te wijzen
*/

#include <Wire.h> // Library voor pins van LCD-display
#include <Adafruit_NeoPixel.h> // Library voor Neopixel
#include <Servo.h>  // Library voor servomotoren
#include <LiquidCrystal_I2C.h> // Library voor LCD-display

#define drukknop A0 // Definieer drukknop voor analoge inputpin A2
#define neopin1 11 // Definieer neopixel LED 1 voor outputpin 11
#define neopin2 10 // Definieer neopixel LED 2 voor outputpin 10
#define joyX A1 //Definieer X-as van joystick voor analoge inputpin A0
#define joyY A2 //Definieer Y-as van joystick voor analoge inputpin A1

Adafruit_NeoPixel neoA = Adafruit_NeoPixel(1, neopin1, NEO_GRB + NEO_KHZ800);
// Aanmaken van een neopixel genaamd neoA, aangesloten op neopin1
Adafruit_NeoPixel neoB = Adafruit_NeoPixel(1, neopin2, NEO_GRB + NEO_KHZ800);
// Aanmaken van een neopixel genaamd neoB, aangesloten op neopin2
LiquidCrystal_I2C lcd(0x27, 16, 2);
// LCD adres definiëren voor een scherm met 16 kolommen en 2 rijen

Servo ServoX;
Servo ServoY;

int sensorArray[] = {A3, A7, A8, A9, A10}; // Definieer een array voor de inputs van de lichtsensoren
int lichtArray[5] = {}; // Definieer een array voor de waarden van de lichtsensoren
int licht;  // Creëer een variabele om de uitgelezen waarde van een lichtsensor te bewaren
int teller; // Creëer een variabele om als teller voor for loops te gebruiken
int joyWaardeX;
int joyWaardeY;
int knopstat; // Creëer variabele voor de status van de drukknop (LOW of HIGH)
int powerup; // Creëer variabele voor de mogelijke powerups
unsigned long tijdNu; // Creëer variabele tijdNu die gebruikt wordt om de eerste millis()-waarde in op te slaan
unsigned long tijdNet; // Creëer variabele tijdNet die gebruikt wordt om de tweede millis()-waarde in op te slaan
long roodA; // Creëer variabele voor rode kleur voor Neopixel A
long groenA; // Creëer variabele voor groene kleur voor Neopixel A
long roodB; // Creëer variabele voor rode kleur voor Neopixel B
long groenB; // Creëer variabele voor groene kleur voor Neopixel B
long zwartA; // Creëer variabele om Neopixel A uit te schakelen
long zwartB; // Creëer variabele om Neopixel B uit te schakelen
long oranjeA; // Creëer variabele voor oranje kleur voor Neopixel A
long oranjeB; // Creëer variabele voor oranje kleur voor Neopixel B
int servoMaxX; // Creëer variabelen om minimum- en maximumwaarden van het aantal graden van de servo's in
int servoMinX; // X en Y richting te bepalen
int servoMaxY;
int servoMinY;
int poortWaarde; // Creëer variabele voor ofwel groene (Winst) ofwel rode (Verlies) poort
int sTijd; // Creëer variabele om begintijd van een ronde in op te slaan voor scoremultiplier op einde
int eTijd; // Creëer variabele om eindtijd van een ronde in op te slaan voor scoremultiplier op einde
int eind; // Creëer variabele om de eindstand van een ronde in op te slaan (Winst of Verlies)
int ronde = 0; // Creëer variabele om te bepalen of er al een vorige ronde is gespeeld (begin bij 0 voor geen vorige ronde)
float tijdMultiplier; // Creëer variabele om multiplier voor score op basis van verstreken milliseconden per
// ronde op te slaan
float score; // Creëer variabele om de float-score van de huidige ronde in op te slaan
unsigned long sScore; //Creëer variabele om vereenvoudigde score (natuurlijk getal) in op te slaan
unsigned long tScore; // Creëer variabele om de totaalscore in op te slaan

void setup() {

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(drukknop, INPUT); // Stel de drukknop in als input
  ServoX.attach(3);
  ServoY.attach(5);
  neoA.begin(); // Initialiseer Neopixel 1
  neoB.begin(); // Initialiseer Neopixel 2
  roodA = neoA.Color(150, 0, 0);
  roodB = neoB.Color(150, 0, 0);
  groenA = neoA.Color(0, 100, 0);
  groenB = neoB.Color(0, 100, 0);
  oranjeA = neoA.Color(150, 100, 0);
  oranjeB = neoB.Color(150, 100, 0);
  zwartA = neoA.Color(0, 0, 0);
  zwartB = neoB.Color(0, 0, 0);
  randomSeed(analogRead(6));

}

void loop() {

  knopstat = digitalRead(drukknop);
  lcd.clear();

  while (knopstat == LOW && ronde == 0) {
    /* Alle commando's binnen deze while loop hebben als doel
      twee verschillende teksten op het LCD-scherm te tonen met een interval van 3 seconden.
      De millis() functie wordt gebruikt zodat de gebruiker op ieder moment op de drukknop
      kan drukken zonder dat het volledige programma onderbroken wordt door een delay().
      Deze tekst wordt enkel tijdens het initiële starten van het programma getoond, na een
      eerste ronde wordt deze tekst overgeslagen.*/

    tijdNu = millis();

    if (tijdNu - tijdNet < 2000) {
      lcd.setCursor(0, 0);
      lcd.print("Welkom bij het");
      lcd.setCursor(0, 1);
      lcd.print("Knikkerlabyrint!");
      neoA.setPixelColor(0, oranjeA);
      neoB.setPixelColor(0, oranjeB);
      neoA.show();
      neoB.show();     

    }

    if (tijdNu - tijdNet == 4000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Druk op de knop");
      lcd.setCursor(0, 1);
      lcd.print("om te starten!");
      neoA.setPixelColor(0, zwartA);
      neoB.setPixelColor(0, zwartB);
      neoA.show();
      neoB.show();

    }

    if (tijdNu - tijdNet >= 8000) {
      lcd.clear();
      tijdNet = millis();

    }
    ServoX.write(90);
    ServoY.write(90);
    knopstat = digitalRead(drukknop);
  }

  while (knopstat == HIGH) {

    knopstat = digitalRead(drukknop);
    ServoX.write(90);
    ServoY.write(90);

  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Spel start over");

  for (teller = 5; teller > 0; teller --) { // Deze for loop telt 5 seconden op de LCD af vóór het spel start

    lcd.setCursor(0, 1);
    lcd.print(teller);
    ServoX.write(90);
    ServoY.write(90);

    if (teller > 1) {

      lcd.setCursor(2, 1);
      lcd.print("seconden!");

    }

    else {

      lcd.setCursor(2, 1);
      lcd.print("seconde! ");

    }

    delay(1000);

  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("START!");

  servoMaxX = 120; // Definieer de graden die de servo kan afleggen wanneer de joystick bediend wordt
  servoMinX = 60;   // Deze waarden zijn variabelen opdat een powerup de besturing van de servo's kan
  servoMaxY = 170;  // omkeren.
  servoMinY = 110;

  poortWaarde = random(0, 2);
  // Kies een willekeurige waarde (0 tot 1) om de poortkleuren (groen of rood) voor deze ronde te bepalen
  if (poortWaarde == 0) {
    neoA.setPixelColor(0, roodA);
    neoB.setPixelColor(0, groenB);
  }
  else {
    neoA.setPixelColor(0, groenA);
    neoB.setPixelColor(0, roodB);
  }
  neoA.show();
  neoB.show();

  arrayChecker(); // Deze functie bepaalt de waarden van de array voor de lichtsensoren

  sTijd = millis(); // Bepaal de starttijd van de ronde

  while (lichtArray[0] > 800 && lichtArray[1] > 400 && lichtArray[4] > 400) { // Zolang de twee poortsensoren en de sensor
    // onderaan in de trechter ononderbroken belicht worden, is hun waarde > 700 en blijft deze while loop doorgaan.
    // Als de knikker voorbij één van deze sensoren rolt, wordt deze loop onderbroken en wordt de eindstand bepaald.

    arrayChecker();

    joyWaardeX = analogRead(joyX); // Deze commando's zorgen ervoor dat de analoge inputwaarden van de joystick gemapt
      // worden naar de minimum- en maximumwaarden van de servo's. Deze waarden worden opgeslagen in de variabele
      //joyWaardeX (en Y) en die wordt op zijn beurt naar de servo geschreven. Op die manier loopt de servo synchroon
      //met de inputwaarde van de joystick.
    joyWaardeX = map(joyWaardeX, 8, 890, servoMinX, servoMaxX);
    ServoX.write(joyWaardeX);

    joyWaardeY = analogRead(joyY);
    joyWaardeY = map(joyWaardeY, 8, 894, servoMinY, servoMaxY);
    ServoY.write(joyWaardeY);

    if (lichtArray[2] < 300 || lichtArray[3] < 300) { // Deze if statement controleert of de knikker over één van de
      // twee powerup-lichtsensoren in het speelveld rolt

      powerup = random(0, 2); // Bepaal een willekeurige waarde voor de powerup (0 of 1)

      if (powerup == 0) { // Deze powerup keert de besturing van de servo's om
        servoMinX = 120;
        servoMaxX = 60;
        servoMinY = 120;
        servoMaxY = 60;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Jouw Powerup:");
        lcd.setCursor(0, 1);
        lcd.print("Omkering!");

      }

      else { // Deze powerup verandert de poorten van kleur, zodat de speler de bal
        // naar de tegenovergestelde poort moet bewegen. De omkering gebeurt door de poortWaarde, bepaald aan
        // het begin van de ronde, uit te lezen.
        if (poortWaarde == 0) {
          neoA.setPixelColor(0, groenA);
          neoB.setPixelColor(0, roodB);
          neoA.show();
          neoB.show();
          poortWaarde = 1; // Afhankelijk van de oorspronkelijke waarde van poortWaarde wordt deze nu omgedraaid.
        }
        else {
          neoA.setPixelColor(0, roodA);
          neoB.setPixelColor(0, groenB);
          neoA.show();
          neoB.show();
          poortWaarde = 0;
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Jouw Powerup:");
        lcd.setCursor(0, 1);
        lcd.print("Andere poort!");

      }

   }
  }

  eTijd = millis() - sTijd; // De eindtijd wordt bepaald op basis van het huidige aantal milliseconden minus het aantal
  // milliseconden (opgeslagen in sTijd) bij de start van de ronde

  lcd.clear();
  lcd.setCursor(0, 0);
  if (lichtArray[0] <= 800) { // Als de knikker in een valkuil in het speelveld valt, zal deze onmiddellijk in de
    // trechter onderaan rollen en deze lichtsensor triggeren, die de boodschap "VERLIES!" op de display weergeeft 
    lcd.print("VERLIES!");
    eind = 0; // De eindstand wordt opgeslagen in deze variabele voor de verdere scorebepaling
  }

  else if (lichtArray[1] <= 400) { // Als de knikker in een poort rolt, zal deze de lichtsensor activeren en afhankelijk
    // van de eerder bepaalde poortWaarde zal dit een "Win" of "Loss" uitkomst teweeg brengen. 
    if (poortWaarde == 0) {
      lcd.print("OVERWINNING!");
      eind = 1;
    }
    else {
      lcd.print("FOUTE POORT!");
      eind = 0;
    }
  }

  else if (lichtArray[4] <= 400) { // Ook de lichtsensor in de andere poort wordt op een analoge manier gecontroleerd.
    if (poortWaarde == 0) {
      lcd.print("FOUTE POORT!");
      eind = 0;
    }
    else {
      lcd.print("OVERWINNING!");
      eind = 1;
    }
  }

  if (eind = 0) {
    neoA.setPixelColor(0, roodA);
    neoB.setPixelColor(0, roodB);
    neoA.show();
    neoB.show();
  }

  else {
    neoA.setPixelColor(0, groenA);
    neoB.setPixelColor(0, groenB);
    neoA.show();
    neoB.show();
  }

  delay(3000);
  lcd.clear();
  if (eind == 0) { // Als de eindstand 0 is, zullen beide LEDs in de poorten rood oplichten om het verlies extra
    // kracht bij te zetten
    sScore = 0; // De huidige rondescore wordt op 0 gezet
    neoA.setPixelColor(0, roodA);
    neoB.setPixelColor(0, roodB);
    neoA.show();
    neoB.show();
  }

  else {
    tijdMultiplier = 1000. / (float)eTijd; // De tijdsbonus wordt berekend door 1000 te delen door het aantal milliseconden
    // dat deze ronde duurde. Dit resulteert in een kommagetal en om dit op te slaan wordt er gebruik gemaakt van een float.
    score = (float)tijdMultiplier * 1000. + 100.; // De eigenlijke score wordt berekend door bij een vaste score van 100 de
    // tijdsbonus, opgeslagen in "tijdMultiplier", te vermenigvuldigen met 1000 en vervolgens op te tellen. Kortere ronde-
    // tijden geven vanzelfsprekend hogere bonusscores
    sScore = score; // De floatscore wordt vereenvoudigd naar een natuurlijk getal, decimalen zijn namelijk overbodig bij
    // de weergave van een score
    neoA.setPixelColor(0, groenA);
    neoB.setPixelColor(0, groenB);
    neoA.show();
    neoB.show();
  }

  lcd.setCursor(0, 0); // Op het display wordt de huidige score weergegeven door "sScore" te printen
  lcd.print("Huidige score:"); 
  lcd.setCursor(0, 1);
  lcd.print(sScore);

  tScore = sScore + tScore; // De totale score wordt berekend door de huidige score op te tellen bij de totaalscore
  // van de vorige ronde(s). Na de eerste ronde is de totaalscore aan de rechterkant dus gelijk aan 0

  delay(3000);

  lcd.setCursor(0, 0); // De totaalscore "tScore" wordt op de display weergegeven
  lcd.print("Totale score:");
  lcd.setCursor(0, 1);
  lcd.print(tScore);

  delay(3000);

  lcd.clear();

  neoA.setPixelColor(0, zwartA); // Na deze score 3 seconden weergegeven te hebben, wordt het scherm gewist en de Neo-
  // pixels uitgeschakeld
  neoB.setPixelColor(0, zwartB);
  neoA.show();
  neoB.show();

  while (knopstat == LOW) { // Deze while loop blijft van kracht tot de drukknop wordt ingedrukt
    knopstat = digitalRead(drukknop);
    lcd.setCursor(0, 0);
    lcd.print("Druk om verder");
    lcd.setCursor(0, 1);
    lcd.print("te gaan!");
  }

  while (knopstat == HIGH) { // Deze while loop fungeert als een pauze zolang de gebruiker de drukknop ingedrukt houdt
    knopstat = digitalRead(drukknop);
  }

  delay(50);
  knopstat = digitalRead(drukknop);
  ronde = 1; // De variabele ronde wordt gelijkgesteld aan 1 zodat de eerste while loop na deze ronde niet meer doorgaat
}

void arrayChecker() {

  for (teller = 4; teller >= 0; teller --) {

    lichtArray[teller] = analogRead(sensorArray[teller]);
  }

}
