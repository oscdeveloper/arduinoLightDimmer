#include <IRremote.h>
#include <EEPROM.h>

#define BULB_PIN_1 4
#define BULB_PIN_2 5
#define BULB_PIN_3 6

#define INTERRUPT_PIN 2

#define IR_PIN 12

IRrecv irrecv(IR_PIN);
decode_results results;

// start settings
// patrz -> resetSettings()
int memoryAnimationProgram = 0;
const byte scaleDivider = 62;
byte dimming = 150;  // Dimming level (8-150)  8 = ON, 150 = OFF | najniższy poziom, do którego się ściemnia | im więcej tym ciemniej
byte dimmingMax = dimming;
byte dimmingMin = 100; //8 has the brightest light| najwyższy poziom, do którego się rozjaśnia | im mniej tym jaśniej
int animationTime = 20;
volatile byte bulbPinNumber;
boolean bulbDirection; // true - liczy w górę | false - liczy w dół
byte animationProgram = 4;
boolean triggerFlag; 
boolean setupMode; 
boolean interruptOnTrigger = true;
unsigned long currentMillis = 0;
const long interval = 500;
// stop settings

/*
 * Programy:
 * 1) Wszystkie powoli się rozjaśniają i ściemniają | pętla zaczyna od zgaszonych żarówek
 * 2) Od lewej do prawej pojedyncza żarówka się rozjasnia i ściemnia | pętla zaczyna ponownie od lewej żarówki
 * 3) Od prawej do lewej pojedyncza żarówka się rozjasnia i ściemnia | pętla zaczyna ponownie od prawej żarówki
 * 4) Wszystkie powoli się rozjaśniają do pewnego poziomu i utrzymują go | brak pętli
 * 5) Od lewej do prawej jedna za drugą się rozjaśnia | pętla zaczyna ponownie od ostatnio ściemnionej żarówki
 * 
 * x) Sterowanie jasnością każdej żarówki pojedynczo
 */


void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();

  if ( EEPROM.read(memoryAnimationProgram) > 0 ) {
    animationProgram = EEPROM.read(memoryAnimationProgram);
  }
  
  pinMode(BULB_PIN_1, OUTPUT);
  pinMode(BULB_PIN_2, OUTPUT);
  pinMode(BULB_PIN_3, OUTPUT);

  interruptOn();
  changeBulbPin();
}




void loop() {

  if (irrecv.decode(&results)) {
   
    interruptOff();

    Serial.print("key decode: ");
    Serial.println(results.value);   
    
    switch (results.value) {
      case 3772784863:
        animationProgram = 1;
      break;
      case 3772817503:
        animationProgram = 2;
      break;
      case 3772801183:
        animationProgram = 3;
      break;
      case 3772780783:
        animationProgram = 4;
      break;  
      case 3772813423:
        animationProgram = 5;
      break; 
      case 3772833823:
        changeBrightness(true); // max poziom jasności w górę
      break; 
      case 3772829743:
        changeBrightness(false); // max poziom jasności w dół
      break; 
      case 3772795063:
        changeDimming(true); // max poziom ciemności w górę
      break; 
      case 3772778743:
        changeDimming(false); // max poziom ciemności w dół
      break;
      case 3772778233:
        changeAnimationTime(true); // prędkość animacji szybciej
      break; 
      case 3772810873:
        changeAnimationTime(false); // prędkość animacji wolniej
      break; 
      case 3772782313:
        setupMode = false; // zapamiętanie settings i wyjście z ustawień
      break;       
    }

    EEPROM.write(memoryAnimationProgram, animationProgram);
    
    changeBulbPin();
    irrecv.resume();
  }

  
  if (!setupMode && (millis() - currentMillis >= interval)) {
    interruptOn();  
//    Serial.println(triggerFlag); 

    if (animationProgram >= 1 && animationProgram <= 3 || animationProgram == 5) {
  
      for (int i=dimmingMax; i>=dimmingMin; i--) { // rozjasnianie
        dimming = i;
        delay(animationTime);
      }
      
      for (int i=dimmingMin; i<=dimmingMax; i++) { // sciemnianie
        dimming = i;
        delay(animationTime);
      } 
    
      if (animationProgram == 2) {
        bulbPinNumber++;
        if (bulbPinNumber > 6) {
          bulbPinNumber = 4;
        }
      } else if (animationProgram == 3) {
        bulbPinNumber--;
        if (bulbPinNumber < 4) {
          bulbPinNumber = 6;
        }    
      } else if (animationProgram == 5) {
        if (bulbPinNumber <= 4) {
          bulbPinNumber++;
          bulbDirection = true;
        } else if (bulbPinNumber >= 6) {
          bulbPinNumber--; 
          bulbDirection = false;
        } else {
          if (bulbDirection) {
            bulbPinNumber++;
          } else {
            bulbPinNumber--;
          }
        }
      }
      
    } else if (animationProgram == 4 && triggerFlag) {
       
      for (byte i=dimmingMax; i>=dimmingMin; i--) { // rozjasnianie
        dimming = i;
        delay(animationTime);
      }  
      triggerFlag = false;
    }
  } // !setupMode
}

void changeBulbPin() {
  if (animationProgram == 2 || animationProgram == 5) {
    bulbPinNumber = 4;
  } else if (animationProgram == 3 || animationProgram == 6) {
    bulbPinNumber = 6;
  }
}

void zeroCrosssInterrupt() {
  // 500 najjasniej
  // 8500 najciemniej, ledwo się żarzy
  // 8000 bardzo ciemno, komfortowo dla oczu, ładnie wygląda

if ( bulbPinNumber == INTERRUPT_PIN ) { // zabezpieczenie, gdyby numer pinu wszedł na pin przerwania
  bulbPinNumber = BULB_PIN_1;
}

  delayMicroseconds(scaleDivider*dimming);

  if (animationProgram == 1 || animationProgram == 4 ) {
    
    digitalWrite(BULB_PIN_1, HIGH);
    digitalWrite(BULB_PIN_2, HIGH);
    digitalWrite(BULB_PIN_3, HIGH);
    delayMicroseconds(10); 
    digitalWrite(BULB_PIN_1, LOW); 
    digitalWrite(BULB_PIN_2, LOW);
    digitalWrite(BULB_PIN_3, LOW);

  } else if (animationProgram == 2 || animationProgram == 3 || animationProgram == 5) {
    
    digitalWrite(bulbPinNumber, HIGH);
    delayMicroseconds(10);
    digitalWrite(bulbPinNumber, LOW);
  
  } else if (animationProgram == 999) {

    digitalWrite(BULB_PIN_3, HIGH);
    delayMicroseconds(10);      
    digitalWrite(BULB_PIN_3, LOW);

    delayMicroseconds(5000);
    digitalWrite(BULB_PIN_2, HIGH);
    delayMicroseconds(10);
    digitalWrite(BULB_PIN_2, LOW);

    delayMicroseconds(1000);
    digitalWrite(BULB_PIN_1, HIGH);
    delayMicroseconds(10);
    digitalWrite(BULB_PIN_1, LOW);
  }
}

void interruptOn() {
  if ( interruptOnTrigger ) {
    resetSettings();
    attachInterrupt(
      digitalPinToInterrupt(INTERRUPT_PIN), 
      zeroCrosssInterrupt,
      RISING
    );    
    interruptOnTrigger = false;
  }
}

void interruptOff() {
  detachInterrupt(
    digitalPinToInterrupt(INTERRUPT_PIN)
  );
  currentMillis = millis();
  setupMode = true;
  triggerFlag = true;
  interruptOnTrigger = true;
  digitalWrite(BULB_PIN_1, HIGH);
}

void resetSettings() {
  triggerFlag = true;
  setupMode = false;
  bulbDirection = true;
  changeBulbPin();
  digitalWrite(BULB_PIN_1, LOW);
  digitalWrite(BULB_PIN_2, LOW);
  digitalWrite(BULB_PIN_3, LOW);
}

void changeBrightness(boolean value) {
  if (value) {
    dimmingMin -= 5;
  } else {
    dimmingMin += 5;
  }
  
  if (dimmingMin<8) {
    dimmingMin = 8;
  } else if (dimmingMin>=dimmingMax) {
    dimmingMin = dimmingMax - 5;
  }
}

void changeDimming(boolean value) {
  if (value) {
    dimmingMax -= 5;
  } else {
    dimmingMax += 5;
  }
  
  if (dimmingMax>150) {
    dimmingMax = 150;
  } else if (dimmingMax<=dimmingMin) {
    dimmingMax = dimmingMin + 5;
  }
}

void changeAnimationTime(boolean value) {
  if (value) {
    animationTime -= 5;
  } else {
    animationTime += 5;
  }

  if (animationTime<5) {
    animationTime = 5;
  }
}

