/*

AC Voltage dimmer with Zero cross detection
Author: Charith Fernanado Adapted by DIY_bloke
License: Creative Commons Attribution Share-Alike 3.0 License.
Attach the Zero cross pin of the module to Arduino External Interrupt pin
Select the correct Interrupt # from the below table 
(the Pin numbers are digital pins, NOT physical pins: 
digital pin 2 [INT0]=physical pin 4 and digital pin 3 [INT1]= physical pin 5)
check: <a href="http://arduino.cc/en/Reference/attachInterrupt">interrupts</a>

Pin    |  Interrrupt # | Arduino Platform
---------------------------------------
2      |  0            |  All -But it is INT1 on the Leonardo
3      |  1            |  All -But it is INT0 on the Leonardo
18     |  5            |  Arduino Mega Only
19     |  4            |  Arduino Mega Only
20     |  3            |  Arduino Mega Only
21     |  2            |  Arduino Mega Only
0      |  0            |  Leonardo
1      |  3            |  Leonardo
7      |  4            |  Leonardo
The Arduino Due has no standard interrupt pins as an iterrupt can be attached to almosty any pin. 

In the program pin 2 is chosen

sprawdzic czy delay dziala w interrupcie
 poczytac o interruptach
*/
#define BULB_PIN_1 3
#define BULB_PIN_2 4
#define BULB_PIN_3 5

#define INTERRUPT_PIN 2

unsigned int dimtime;
unsigned int dimming = 140;  // Dimming level (0-128)  0 = ON, 128 = OFF | najniższy poziom, do którego się ściemnia | im więcej tym ciemniej
unsigned int dimmingMax = dimming;
unsigned int dimmingMin = 40; //50 has the brightest light| najwyższy poziom, do którego się rozjaśnia | im mniej tym jaśniej
unsigned int animationTime = 20;
unsigned int us = 75;
unsigned int bulbPinNumber;
unsigned int animationType = 2;




void setup() {
  
  pinMode(BULB_PIN_1, OUTPUT);
  pinMode(BULB_PIN_2, OUTPUT);
  pinMode(BULB_PIN_3, OUTPUT);
  
  digitalWrite(BULB_PIN_1, LOW);
  digitalWrite(BULB_PIN_2, LOW);
  digitalWrite(BULB_PIN_3, LOW);
  
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), zeroCrosssInterrupt, RISING);

  if (animationType == 2) {
    bulbPinNumber = 3;
  } else if (animationType == 3) {
    bulbPinNumber = 5;
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
// 3000 najjasniej
// 10500 najciemniej
  
  dimtime = (us*dimming);    // For 60Hz => 65    
  //dimtime = 10500; 
  delayMicroseconds(dimtime);    // Wait till firing the TRIAC  

  if (animationType == 1 || animationType == 4 || animationType == 5 || animationType == 6) {
    digitalWrite(BULB_PIN_1, HIGH);   // Fire the TRIAC
    digitalWrite(BULB_PIN_2, HIGH);
    digitalWrite(BULB_PIN_3, HIGH);
    delayMicroseconds(10);         // triac On propogation delay 
    digitalWrite(BULB_PIN_1, LOW);    // No longer trigger the TRIAC (the next zero crossing will swith it off)
    digitalWrite(BULB_PIN_2, LOW);
    digitalWrite(BULB_PIN_3, LOW);
  } else if (animationType == 2 || animationType == 3) {
    digitalWrite(bulbPinNumber, HIGH);
    delayMicroseconds(10);
    digitalWrite(bulbPinNumber, LOW);
  }

}




void loop()  {

  if (animationType >= 1  && animationType <= 3) {
  
    for (int i=dimmingMax; i >= dimmingMin; i--) { // rozjasnianie
      dimming = i;
      delay(animationTime);
    }
    
    for (int i=dimmingMin; i <= dimmingMax; i++) { // sciemnianie
      dimming = i;
      delay(animationTime);
    } 
  
    if (animationType == 2) {
      bulbPinNumber++;
      if (bulbPinNumber > 5) {
        bulbPinNumber = 3;
      }
    } else if (animationType == 3) {
      bulbPinNumber--;
      if (bulbPinNumber < 3) {
        bulbPinNumber = 5;
      }    
    }
  } else if (animationType == 4) {
    dimming = dimmingMax;
    delay(animationTime);
  } else if (animationType == 5) {
    dimming = dimmingMax / 2;
    delay(animationTime);
  } else if (animationType == 6) {
    dimming = dimmingMax / 1.2;
    delay(animationTime);
  }
}
