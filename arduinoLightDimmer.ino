#define BULB_PIN_1 4
#define BULB_PIN_2 5
#define BULB_PIN_3 6

#define INTERRUPT_PIN 2

const byte scaleDivider = 62;
unsigned int dimtime; // czas w mikrosekundach, wynik mnożenia scaleDivider*dimming
byte dimming = 150;  // Dimming level (8-128)  8 = ON, 150 = OFF | najniższy poziom, do którego się ściemnia | im więcej tym ciemniej
byte dimmingMax = dimming;
byte dimmingMin = 100; //8 has the brightest light| najwyższy poziom, do którego się rozjaśnia | im mniej tym jaśniej
unsigned int animationTime = 20;
byte bulbPinNumber;
boolean bulbDirection = true; // true - liczy w górę | false - liczy w dół
const byte animationProgram = 5;
boolean triggerFlag = true; 
volatile boolean triggerFlagInterrupt = true; 

/*
 * Programy:
 * 1) Wszystkie powoli się rozjaśniają i ściemniają
 * 2) Od lewej do prawej pojedyncza żarówka się rozjasnia i ściemnia
 * 3) Od prawej do lewej pojedyncza żarówka się rozjasnia i ściemnia
 * 4) Wszystkie powoli się rozjaśniają do pewnego poziomu i utrzymują go
 * 5) Od lewej do prawej jedna za drugą się rozjaśnia 
 * 
 * x) Sterowanie jasnością każdej żarówki pojedynczo
 */


void setup() {

  pinMode(BULB_PIN_1, OUTPUT);
  pinMode(BULB_PIN_2, OUTPUT);
  pinMode(BULB_PIN_3, OUTPUT);
  
  digitalWrite(BULB_PIN_1, LOW);
  digitalWrite(BULB_PIN_2, LOW);
  digitalWrite(BULB_PIN_3, LOW);

  attachInterrupt(
    digitalPinToInterrupt(INTERRUPT_PIN), 
    zeroCrosssInterrupt,
    RISING
  );

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

  dimtime = scaleDivider*dimming; 
//  dimtime = 8500; 
  delayMicroseconds(dimtime);

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




void loop()  {

  if (animationProgram >= 1  && animationProgram <= 3 || animationProgram == 5) {

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
    
  } else if (animationProgram == 4) {

    if ( triggerFlag ) {
      for (byte i=dimmingMax; i>=dimmingMin; i--) { // rozjasnianie
        dimming = i;
        delay(animationTime);
      }  
      triggerFlag = false;  
    }   
  }
}
