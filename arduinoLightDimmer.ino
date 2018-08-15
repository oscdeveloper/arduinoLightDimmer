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
// take a look -> resetSettings()
byte memoryAnimationProgram = 0;
byte memoryDimmingMax = 1;
byte memoryDimmingMin = 2;
byte memoryAnimationTime = 3;

const byte scaleDivider = 62;
byte dimming = 150;  // Dimming level (8-150)  8 = ON, 150 = OFF | the lowest level of dimming | bigger number = darker light
byte dimmingMax = dimming;
byte dimmingMin = 100; // 8 has the brightest light | the highest level of brightness | smaller number = brigther light
int animationTime = 20;
volatile byte bulbPinNumber;
boolean bulbDirection; // true - count up | false - count down
byte animationProgram = 4;
boolean triggerFlag; 
boolean setupMode; 
boolean interruptOnTrigger = true;
unsigned long currentMillis = 0;
const long interval = 500;
volatile int firstTrigger = 0;
// stop settings

/*
 * Programs:
 * 1) All bulbs lights up and dim in loop | loop begins from totally dimmed light
 * 2) From left to right one after another every single bulb lights up and dim | loop begins again from left bulb
 * 3) From right to left one after another every single bulb lights up and dim | loop begins again from right bulb
 * 4) All bulbs lights up to defined by user level and they hold this state | no loop
 * 5) From left to right one after another every single bulb lights up and dim | loop begins from last dimmed bulb
 */


void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();

  if ( EEPROM.read(memoryAnimationProgram) < 255 ) {
    animationProgram = EEPROM.read(memoryAnimationProgram);
  }

  if ( EEPROM.read(memoryDimmingMax) < 255 ) {
    dimmingMax = EEPROM.read(memoryDimmingMax);
  }  

  if ( EEPROM.read(memoryDimmingMin) < 255 ) {
    dimmingMin = EEPROM.read(memoryDimmingMin);
  }    

  if ( EEPROMReadlong(memoryAnimationTime) > -1 ) {
    animationTime = EEPROMReadlong(memoryAnimationTime);
  }
  
  pinMode(BULB_PIN_1, OUTPUT);
  pinMode(BULB_PIN_2, OUTPUT);
  pinMode(BULB_PIN_3, OUTPUT);

  interruptOn();
}




void loop() {

  if (irrecv.decode(&results)) { // receive IR signal from TV remote controller
   
    interruptOff(); // switch off interrupt, it's collides with IR library

    //Serial.print("key decode: ");Serial.println(results.value);   
    
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
        changeBrightness(true); // max level of brightness up max
      break; 
      case 3772829743:
        changeBrightness(false); // max level of brightness down
      break; 
      case 3772795063:
        changeDimming(true); // max level of dimming up
      break; 
      case 3772778743:
        changeDimming(false); // max level of dimming down
      break;
      case 3772778233:
        changeAnimationTime(true); // animation speed faster
      break; 
      case 3772810873:
        changeAnimationTime(false); // animation speed slower
      break; 
      case 3772782313:
        setupMode = false; // save settings and exit from setup mode
      break;       
    }

    EEPROM.write(memoryAnimationProgram, animationProgram);
    EEPROM.write(memoryDimmingMax, dimmingMax);
    EEPROM.write(memoryDimmingMin, dimmingMin);
    EEPROMWritelong(memoryAnimationTime, animationTime);
    
    changeBulbPin();
    irrecv.resume();
  }

  
  if (!setupMode && (millis() - currentMillis >= interval)) {
    
    interruptOn();  

    if (animationProgram >= 1 && animationProgram <= 3 || animationProgram == 5) {
  
      for (int i=dimmingMax; i>=dimmingMin; i--) { // brighter
        dimming = i;
        delay(animationTime);
      }
      
      for (int i=dimmingMin; i<=dimmingMax; i++) { // darker
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
       
      for (byte i=dimmingMax; i>=dimmingMin; i--) { // brightening
        dimming = i;
        delay(animationTime);
      }  
      triggerFlag = false;
    }
  } // !setupMode
}




void zeroCrosssInterrupt() {

  if ( bulbPinNumber == INTERRUPT_PIN ) { // protection if bulb pin number goes to interrupt pin number
    bulbPinNumber = BULB_PIN_1;
  }

  delayMicroseconds(scaleDivider*dimming);
   
  if (firstTrigger > 10) { // remove bulb flash just after start when interrupt is attached
    fireTriac();
  } else {
    firstTrigger++;
  }
}




void fireTriac() {
  if (animationProgram == 1 || animationProgram == 4) {
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
  }
}




void changeBulbPin() {
  if (animationProgram == 2 || animationProgram == 5) {
    bulbPinNumber = 4;
  } else if (animationProgram == 3 || animationProgram == 6) {
    bulbPinNumber = 6;
  }
}




void interruptOn() {
  if (interruptOnTrigger) {
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

void EEPROMWritelong(int address, long value) {
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

long EEPROMReadlong(long address) {
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
