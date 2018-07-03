#define BULB_PIN_1 4
#define BULB_PIN_2 5
#define BULB_PIN_3 6

#define INTERRUPT_PIN 2

const byte us = 75;
unsigned int dimtime;
byte dimming = 149;  // 140 |  Dimming level (0-128)  0 = ON, 128 = OFF | najniższy poziom, do którego się ściemnia | im więcej tym ciemniej
byte dimmingMax = dimming;
byte dimmingMin = 50; //50 has the brightest light| najwyższy poziom, do którego się rozjaśnia | im mniej tym jaśniej
unsigned int animationTime = 100;
byte bulbPinNumber;
const byte animationType = 1;
boolean triggerFlag = true;

/*
 * Programy:
 * 1) Wszystkie powoli się rozjaśniają i ściemniają
 * 2) Od lewej do prawej pojedyncza żarówka się rozjasnia i ściemnia
 * 3) Od prawej do lewej pojedyncza żarówka się rozjasnia i ściemnia
 * 4) Wszystkie powoli się rozjaśniają do pewnego poziomu i utrzymują go
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

  if (animationType == 2) {
    bulbPinNumber = 4;
  } else if (animationType == 3) {
    bulbPinNumber = 6;
  }  
}

//the interrupt function must take no parameters and return nothing
void zeroCrosssInterrupt() {
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing thus: (50Hz)-> 10ms (1/2 Cycle) 
  // For 60Hz => 8.33ms (10.000/120)
  // 10ms=10000us
  // (10000us - 10us) / 128 = 75 (Approx) For 60Hz => 65
  /*
   * 9990 / 50 = 199 us
   * 9990 / 40 = 249 us
   * 
   */
  // 500 najjasniej
  // 8500 najciemniej, ledwo się żarzy
  // 
  
  //dimtime = (us*dimming);    // For 60Hz => 65
  
  dimtime = 8000; 
  delayMicroseconds(dimtime);    // Wait till firing the TRIAC  

  if (animationType == 1 || animationType == 4 ) {
    
//    digitalWrite(BULB_PIN_1, HIGH);   // Fire the TRIAC
//    digitalWrite(BULB_PIN_2, HIGH);
    digitalWrite(BULB_PIN_3, HIGH);
    delayMicroseconds(10);         // triac On propogation delay 
//    digitalWrite(BULB_PIN_1, LOW);    // No longer trigger the TRIAC (the next zero crossing will swith it off)
//    digitalWrite(BULB_PIN_2, LOW);
    digitalWrite(BULB_PIN_3, LOW);

//    delayMicroseconds(1000);
//    digitalWrite(BULB_PIN_2, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(BULB_PIN_2, LOW);
//
//    delayMicroseconds(1000);
//    digitalWrite(BULB_PIN_3, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(BULB_PIN_3, LOW);
    
  } else if (animationType == 2 || animationType == 3) {
    
    digitalWrite(bulbPinNumber, HIGH);
    delayMicroseconds(10);
    digitalWrite(bulbPinNumber, LOW);
  
  }

}




void loop()  {
delay(100);
//  if (animationType >= 1  && animationType <= 3) {
  if (animationType == 9) {
    for (int i=dimmingMax; i>=dimmingMin; i--) { // rozjasnianie
      dimming = i;
      delay(animationTime);
    }
    
    for (int i=dimmingMin; i<=dimmingMax; i++) { // sciemnianie
      dimming = i;
      delay(animationTime);
    } 
  
    if (animationType == 2) {
      bulbPinNumber++;
      if (bulbPinNumber > 6) {
        bulbPinNumber = 4;
      }
    } else if (animationType == 3) {
      bulbPinNumber--;
      if (bulbPinNumber < 4) {
        bulbPinNumber = 6;
      }    
    }
    
  } else if (animationType == 4) {
//Dane:
//unsigned int dimming = 149;
//unsigned int animationTime = 100;

    if ( triggerFlag ) {
      for (int i=dimmingMax; i>=dimmingMax/2; i--) { // rozjasnianie
        dimming = i;
        delay(animationTime);
      }  
      triggerFlag = false;  
    }   
  }
}
