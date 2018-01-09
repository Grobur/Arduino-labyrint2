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

#define drukknop A2 // Definieer drukknop voor analoge inputpin A2
#define neopin1 11 // Definieer neopixel LED 1 voor outputpin 11
#define neopin2 10 // Definieer neopixel LED 2 voor outputpin 10
#define joyX A0 //Definieer X-as van joystick voor analoge inputpin A0
#define joyY A1 //Definieer Y-as van joystick voor analoge inputpin A1

Adafruit_NeoPixel neoA = Adafruit_NeoPixel(1, neopin1, NEO_GRB + NEO_KHZ800);
// Aanmaken van een neopixel genaamd neoA, aangesloten op neopin1
Adafruit_NeoPixel neoB = Adafruit_NeoPixel(1, neopin2, NEO_GRB + NEO_KHZ800);
// Aanmaken van een neopixel genaamd neoB, aangesloten op neopin2
LiquidCrystal_I2C lcd(0x27, 16, 2);
// LCD adres definiëren voor een scherm met 16 kolommen en 2 rijen

Servo ServoX;
Servo ServoY;

int sensorArray[] = {A3, A7, A8, A9, A10}; // Definieer een array voor de inputs van de lichtsensoren
int lichtArray[] = {1, 2, 3, 4, 5}; // Definieer een array voor de waarden van de lichtsensoren
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
int servoMaxX;
int servoMinX;
int servoMaxY;
int servoMinY;
int poortWaarde; // Creëer variabele voor ofwel groene (Winst) ofwel rode (Verlies) poort

void setup() {

  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  pinMode(drukknop, INPUT); // Stel de drukknop in als input
  ServoX.attach(3);
  ServoY.attach(5);
  neoA.begin(); // Initialiseer Neopixel 1
  neoB.begin(); // Initialiseer Neopixel 2
  roodA = neoA.Color(100, 0, 0);
  groenA = neoA.Color(0, 100, 0);
  roodB = neoB.Color(100, 0, 0);
  groenB = neoB.Color(0, 100, 0);
  randomSeed(analogRead(6));

}

void loop() {

  knopstat = digitalRead(drukknop);
  lcd.clear();

  while (knopstat == LOW) {
    /* Alle commando's binnen deze while loop hebben als doel
      twee verschillende teksten op het LCD-scherm te tonen met een interval van 3 seconden.
      De millis() functie wordt gebruikt zodat de gebruiker op ieder moment op de drukknop
      kan drukken zonder dat het volledige programma onderbroken wordt door een delay().*/

    tijdNu = millis();

    if (tijdNu - tijdNet < 2000) {
      lcd.setCursor(0, 0);
      lcd.print("Welkom bij het");
      lcd.setCursor(0, 1);
      lcd.print("Knikkerlabyrint!");
      Serial.println(tijdNu);

    }

    if (tijdNu - tijdNet == 4000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Druk op de knop");
      lcd.setCursor(0, 1);
      lcd.print("om te starten!");

    }

    if (tijdNu - tijdNet >= 8000) {
      lcd.clear();
      tijdNet = millis();

    }

    knopstat = digitalRead(drukknop);
  }

  while (knopstat == HIGH) {

    knopstat = digitalRead(drukknop);

  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Spel start over");

  for (teller = 5; teller > 0; teller --) {

    lcd.setCursor(0, 1);
    lcd.print(teller);

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

  servoMaxX = 120; // Definieer de graden die de servo kan afleggen wanneer de joystick bediend wordt
  servoMinX = 60;   // Deze waarden zijn variabelen opdat een powerup de besturing van de servo's kan
  servoMaxY = 120;  // omkeren.
  servoMinY = 60;

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

  while (lichtArray[1] > 500 && lichtArray[2] > 700 && lichtArray[5] > 700) {

    for (teller = 5; teller > 0; teller --) {

      lichtArray[teller] = analogRead(sensorArray[teller]);

      joyWaardeX = analogRead(joyX);
      joyWaardeX = map(joyWaardeX, 8, 890, servoMinX, servoMaxX);
      ServoX.write(joyWaardeX);

      joyWaardeY = analogRead(joyY);
      joyWaardeY = map(joyWaardeY, 8, 894, servoMinY, servoMaxY);
      ServoY.write(joyWaardeY);

      if (lichtArray[3] < 700 || lichtArray[4] < 700) {

        powerup = random(0, 2); // Bepaal een willekeurige waarde voor de powerup (0 of 1)

        lcd.setCursor(0, 0);
        lcd.print("Jouw Powerup:")
        if (powerup == 0) { // Deze powerup keert de besturing van de servo's om
          servoMinX = 120;
          servoMaxX = 60;
          servoMinY = 120;
          servoMaxY = 60;
          lcd.setCursor(0, 1);
          lcd.print("Omkering!");
        }

        else { // Deze powerup verandert de poorten van kleur, zodat de speler de bal
          // naar de tegenovergestelde poort moet bewegen
          if (poortWaarde == 0) {
            neoA.setPixelColor(0, groenA);
            neoB.setPixelColor(0, roodB);
          }
          else {
            neoA.setPixelColor(0, roodA);
            neoB.setPixelColor(0, groenB);
          }

          lcd.setCursor(0, 1);
          lcd.print("Andere poort!")

        }
      }
    }

  }

  if (lichtArray < 500) {

  }

  knopstat = digitalRead(drukknop);
  delay(50);
  tijdNet = millis();
}







