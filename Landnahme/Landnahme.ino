 /*
 * MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY      COOQROBOT.
 * The library file MFRC522.h has a wealth of useful info. Please read it.
 * The functions are documented in MFRC522.cpp.
 *
 * Based on code Dr.Leong   ( WWW.B2CQSHOP.COM )
 * Created by Miguel Balboa (circuitito.com), Jan, 2012.
 * Rewritten by Søren Thing Andersen (access.thing.dk), fall of 2013 (Translation to      English, refactored, comments, anti collision, cascade levels.)
 * Released into the public domain.
 * Sample program showing how to read data from a PICC using a MFRC522 reader on the      Arduino SPI interface.
 *----------------------------------------------------------------------------- empty_skull 
 * Aggiunti pin per arduino Mega
 * add pin configuration for arduino mega
 * http://mac86project.altervista.org/
 ----------------------------------------------------------------------------- Nicola      Coppola
 * Pin layout should be as follows:
 * Signal     Pin              Pin               Pin
 *            Arduino Uno      Arduino Mega      MFRC522 board
 * ------------------------------------------------------------
 * Reset      9                5                 RST
 * SPI SS     10               53                SDA
 * SPI MOSI   11               52                MOSI
 * SPI MISO   12               51                MISO
 * SPI SCK    13               50                SCK
 *
 * The reader can be found on eBay for around 5 dollars. Search for "mf-rc522" on    ebay.com. 
 */






// ShiftPWM uses timer1 by default. To use a different timer, before '#include <ShiftPWM.h>', add
#define SHIFTPWM_USE_TIMER2  // for Arduino Uno and earlier (Atmega328)
// #define SHIFTPWM_USE_TIMER3  // for Arduino Micro/Leonardo (Atmega32u4)

// Clock and data pins are pins from the hardware SPI, you cannot choose them yourself.
// Data pin is MOSI (Uno and earlier: 11, Leonardo: ICSP 4, Mega: 51, Teensy 2.0: 2, Teensy 2.0++: 22) 
// Clock pin is SCK (Uno and earlier: 13, Leonardo: ICSP 3, Mega: 52, Teensy 2.0: 1, Teensy 2.0++: 21)

// You can choose the latch pin yourself.
const int ShiftPWM_latchPin=0;


#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 1;
const int ShiftPWM_clockPin = 2;


// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = false;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

// Function prototypes (telling the compiler these functions exist).
void oneByOne(void);
void inOutTwoLeds(void);
void inOutAll(void);
void alternatingColors(void);
void hueShiftAll(void);
void randomColors(void);
void fakeVuMeter(void);
void rgbLedRainbow(unsigned long cycleTime, int rainbowWidth);
void printInstructions(void);

// Here you set the number of brightness levels, the update frequency and the number of shift registers.
// These values affect the load of ShiftPWM.
// Choose them wisely and use the PrintInterruptLoad() function to verify your load.
unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
unsigned int numRegisters = 1;
unsigned int numOutputs = 6;
unsigned int numRGBLeds = 2;
unsigned int fadingMode = 0; //start with all LED's off.

unsigned long startTime = 0; // start time for the chosen fading mode


 



#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>


//die modi in denen sich der Schrein befinden kann
#define real 0
#define demo 1
#define diagnostic 2


//der status in dem sich der schrein befinden kann
#define Free 0
#define activating 1
#define loading 2
#define active 3
#define blocked 4



//pin belegungen. kann je nach aufbau verändert werden.

#define servoLED1_PIN1 3
#define servoLED1_PIN2 4
#define servoLED2_PIN1 5
#define servoLED2_PIN2 6
#define servo_PIN 8
#define RST_PIN 9
#define SS_PIN 10
//9-13 muss frei bleiben für den rfid-sensor#define rgb_blue_PIN2 14



MFRC522 mfrc522(SS_PIN, RST_PIN);

Servo myServo;

#define minuten 60000

//die einzelnen Karten Identifier

#define Antike 0
#define Elben 1
#define Komet 2
#define HDC 3
#define Imperium 4
#define Krone 5
#define Licht 6
#define Norrelag 7
#define OHL 8
#define Pilger 9
#define Zusammenkunft 10
#define Stadt 11
#define Lesath 12
#define Neutral 13
#define Dunkel 14
#define Demo 15
#define Diagnostic 16
#define Unbekannt 17


//die zeiten für die phasen
const unsigned long ta = 10*minuten;      //Aktivierungszeit, Zeit in Phase 1
const unsigned long tl = 60*minuten;      //Ladezeit, Zeit in Phase 2
const unsigned long tn = 10*minuten;      //Landnahmezeit, Zeit in Phase 3
const unsigned long tb = 90*minuten;      //Blockzeit, Zeit in Phase 4
                                          //Phase 0 hat keine Zeit und kann beliebig lange gehen

                                        

short _status;
short newKeeper;
short mode;
long servoValue;
short servoDirection;

unsigned long activeTime;

int failCount;

int scan;

void setup() {
Serial.begin(9600);
  myServo.attach(servo_PIN);
  pinMode(servoLED1_PIN1, OUTPUT);
  pinMode(servoLED1_PIN2, OUTPUT);

    // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);

  // SetPinGrouping allows flexibility in LED setup. 
  // If your LED's are connected like this: RRRRGGGGBBBBRRRRGGGGBBBB, use SetPinGrouping(4).
  ShiftPWM.SetPinGrouping(2); //This is the default, but I added here to demonstrate how to use the funtion
  
  ShiftPWM.Start(pwmFrequency,maxBrightness);  

  _status=Free;
  newKeeper=Neutral;

  servoValue = 90;                             //normal 90
  servoDirection=1;
  myServo.write(servoValue);


  failCount=0;
  mode = 0;

  activeTime=0;

  scan = 0;

  SPI.begin();
  mfrc522.PCD_Init();

  setShrine();

}

void loop() {
  switch(mode)
  {
    case(real):
    switch(_status)
    {
      case(activating):
      if((micros()-activeTime)<ta)
      {
        if(!mfrc522.PICC_IsNewCardPresent())
        {
          failCount++;
          if(failCount>25)
          {
            _status--;
          }
        }
        else
        {
          failCount=0;
        }
      }
      else
      {
        _status++;
        failCount=0;
        activeTime=micros();
        setShrine();
      }
      break;
 
      case(loading):
      pulse();
        if((micros()-activeTime)<tl)
        {
          scan=scanForCard();
          if(scan==Lesath||scan==Neutral||scan==Dunkel)
          {

            newKeeper = scan;
            _status=0;
            setShrine();  
          }
        }
        else
        {
            _status++;
            activeTime=micros();
            setShrine();        
        }

      break;

      case(active):
      pulse();
      if((micros()-activeTime)<tn)
      {
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          newKeeper = scan;
          _status=0;
          setShrine();  
        }
        else if(scan!=Unbekannt)
        {
          newKeeper=scan;
          _status++;
          setShrine();
          activeTime=micros();          
        }
        
      }
      else
      {
        _status=0;
        setShrine();
        activeTime=micros();
      }
      
      break;

      case(blocked):
      if((micros()-activeTime)<tb)
      {
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          newKeeper = scan;
          _status=0;
          activeTime=micros();
          setShrine();  
        }
      }
      else
      {
        _status=0;
        activeTime=micros();
        setShrine();
      }
      break;

      default:
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          newKeeper = scan;
          setShrine();
        }
        else if(scan==demo)
        {
          mode=demo;
        }
        else if(scan==diagnostic)
        {
          mode=diagnostic;
        }
        else if(scan!=Unbekannt)
        {
          _status++;
          failCount=0;
          activeTime=micros();
          setShrine();
        }
      
      return;

    }
    break;

    //demomodus: alle zeiten geteilt durch 10
    case(demo):

    break;

    case(diagnostic):
    pulse();
    if(scanForCard()==diagnostic)
    {
      _status=(_status+1)%5;
      newKeeper=random()%15;
      setShrine();
      delay(3000);      
    }

    break;
  }
}
  

int scanForCard()
{
   if (!mfrc522.PICC_IsNewCardPresent())
    return Unbekannt;                                                            //Unbekannt bedeutet das keine karte gefunden wurde = 16
   else
   {
    String rfidUid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      rfidUid += String(mfrc522.uid.uidByte[i], HEX);
    }

    if(rfidUid == "000704343d")
      return Lesath;
    if(rfidUid == "0002033c00")
      return Antike;
    if(rfidUid == "000704361a")
      return Elben;
    if(rfidUid == "0007050246")
      return Komet;
    if(rfidUid == "00070b1929")
      return HDC;
    if(rfidUid == "000661214d")
      return Imperium;
    if(rfidUid == "0007050455")
      return Krone;
    if(rfidUid == "000704343d")
      return Neutral;
    if(rfidUid == "0007205d05")
      return Licht;
    if(rfidUid == "0006613b3d")
      return Norrelag;
    if(rfidUid == "0007092c5a")
      return OHL;
    if(rfidUid == "0006540c50")
      return Pilger;
    if(rfidUid == "0002034400")
      return Zusammenkunft;
    if(rfidUid == "0006531113")
      return Stadt;
    if(rfidUid == "0007050456")
      return Dunkel;
    if(rfidUid == "0006542540")
      return Demo;
    if(rfidUid == "0006533649")
      return Diagnostic;
      
  }
}

void setServo()
{
  myServo.write(7.5+15*newKeeper);
}

void setShrine()
{

  switch(_status)
  {
    case(activating):

    //RGB AUF BLAU
    ShiftPWM.SetRGB(1,0,0,255);    // blue
    ShiftPWM.SetRGB(2,0,0,255);    // blue
    

    
    if(newKeeper==Lesath||newKeeper==Neutral||newKeeper==Dunkel)
    {
      //WENN SL-LAGER: SERVO-LED AUS
      digitalWrite(servoLED1_PIN1, LOW);
      digitalWrite(servoLED1_PIN2, LOW);

      digitalWrite(servoLED2_PIN1, HIGH);
      digitalWrite(servoLED2_PIN2, HIGH);      
      
      switch(newKeeper)
      {
        case(Lesath):
        myServo.write(30);
        break;
        case(Neutral):
        myServo.write(90);

        break;
        case(Dunkel):
        myServo.write(150);

        break;
        
      }
    }
    else
    {

      //SERVO LEDs AN
      digitalWrite(servoLED1_PIN1, HIGH);
      digitalWrite(servoLED1_PIN2, HIGH);

      digitalWrite(servoLED2_PIN1, LOW);
      digitalWrite(servoLED2_PIN2, LOW); 
    }
    break;
    
    case(loading):
    //servo leds an
    digitalWrite(servoLED1_PIN1, HIGH);
    digitalWrite(servoLED1_PIN2, HIGH);
    
    digitalWrite(servoLED2_PIN1, LOW);
    digitalWrite(servoLED2_PIN2, LOW); 
    
    break;

    case(active):
    

    break;

    case(blocked):
    //servo auf position setzen

    
    //RGB auf Rot setzen
    ShiftPWM.SetRGB(1,255,0,0);    // red
    ShiftPWM.SetRGB(2,255,0,0);    // red

    
    if(newKeeper==Lesath||newKeeper==Neutral||newKeeper==Dunkel)
    {
      digitalWrite(servoLED1_PIN1, LOW);
      digitalWrite(servoLED1_PIN2, LOW);
      digitalWrite(servoLED2_PIN1, HIGH);
      digitalWrite(servoLED2_PIN2, HIGH); 
      switch(newKeeper)
      {
        case(Lesath):
        myServo.write(30);
        break;
        case(Neutral):
        myServo.write(90);

        break;
        case(Dunkel):
        myServo.write(150);

        break;        
      }
    }
    else
    {
      //Servo-LEDs an, SL-LEDs aus
      digitalWrite(servoLED1_PIN1, HIGH);
      digitalWrite(servoLED1_PIN2, HIGH);
      digitalWrite(servoLED2_PIN1, LOW);
      digitalWrite(servoLED2_PIN2, LOW);
      setServo();
    }
    
    break;

    default:
    //RGB auf Grün setzen
    ShiftPWM.SetRGB(1,0,255,0);    // green
    ShiftPWM.SetRGB(2,0,255,0);    // green

    
    if(newKeeper==Lesath||newKeeper==Neutral||newKeeper==Dunkel)
    {
      digitalWrite(servoLED1_PIN1, LOW);
      digitalWrite(servoLED1_PIN2, LOW);
      digitalWrite(servoLED2_PIN1, HIGH);
      digitalWrite(servoLED2_PIN2, HIGH);
    }
    else
    {
      //Servo-LEDs an, SL-LEDs aus
      digitalWrite(servoLED1_PIN1, HIGH);
      digitalWrite(servoLED1_PIN2, HIGH);
      digitalWrite(servoLED2_PIN1, LOW);
      digitalWrite(servoLED2_PIN2, LOW);      
    }
   return;     
  }
  return;
}

void pulse()
{

  switch(_status)
  {
    case(loading):
      oneByOne();
      myServo.write(servoValue);
      delay(5);
      servoValue+=servoDirection;
      if(servoValue%180==0)
        servoDirection*=-1;

    break;

    case(active):
    hueShiftAll();
      myServo.write(servoValue);
      delay(5);
      servoValue+=servoDirection;
      if(servoValue%180==0)
        servoDirection*=-1;
  return;
}
}

void oneByOne(void){ // Fade in and fade out all outputs one at a time
  unsigned char brightness;
  unsigned long fadeTime = 500;
  unsigned long loopTime = numOutputs*fadeTime*2;
  unsigned long time = millis()-startTime;
  unsigned long timer = time%loopTime;
  unsigned long currentStep = timer%(fadeTime*2);

  int activeLED = timer/(fadeTime*2);

  if(currentStep <= fadeTime ){
    brightness = currentStep*maxBrightness/fadeTime; ///fading in
  }
  else{
    brightness = maxBrightness-(currentStep-fadeTime)*maxBrightness/fadeTime; ///fading out;
  }
  ShiftPWM.SetAll(0);
  ShiftPWM.SetOne(activeLED, brightness);
}

void inOutTwoLeds(void){ // Fade in and out 2 outputs at a time
  unsigned long fadeTime = 500;
  unsigned long loopTime = numOutputs*fadeTime;
  unsigned long time = millis()-startTime;
  unsigned long timer = time%loopTime;
  unsigned long currentStep = timer%fadeTime;

  int activeLED = timer/fadeTime;
  unsigned char brightness = currentStep*maxBrightness/fadeTime;

  ShiftPWM.SetAll(0);
  ShiftPWM.SetOne((activeLED+1)%numOutputs,brightness);
  ShiftPWM.SetOne(activeLED,maxBrightness-brightness);
}

void inOutAll(void){  // Fade in all outputs
  unsigned char brightness;
  unsigned long fadeTime = 2000;
  unsigned long time = millis()-startTime;
  unsigned long currentStep = time%(fadeTime*2);

  if(currentStep <= fadeTime ){
    brightness = currentStep*maxBrightness/fadeTime; ///fading in
  }
  else{
    brightness = maxBrightness-(currentStep-fadeTime)*maxBrightness/fadeTime; ///fading out;
  }
  ShiftPWM.SetAll(brightness);
}

void alternatingColors(void){ // Alternate LED's in 6 different colors
  unsigned long holdTime = 2000;
  unsigned long time = millis()-startTime;
  unsigned long shift = (time/holdTime)%6;
  for(unsigned int led=0; led<numRGBLeds; led++){
    switch((led+shift)%6){
    case 0:
      ShiftPWM.SetRGB(led,255,0,0);    // red
      break;
    case 1:
      ShiftPWM.SetRGB(led,0,255,0);    // green
      break;
    case 2:
      ShiftPWM.SetRGB(led,0,0,255);    // blue
      break;
    case 3:
      ShiftPWM.SetRGB(led,255,128,0);  // orange
      break;
    case 4:
      ShiftPWM.SetRGB(led,0,255,255);  // turqoise
      break;
    case 5:
      ShiftPWM.SetRGB(led,255,0,255);  // purple
      break;
    }
  }
}

void hueShiftAll(void){  // Hue shift all LED's
  unsigned long cycleTime = 10000;
  unsigned long time = millis()-startTime;
  unsigned long hue = (360*time/cycleTime)%360;
  ShiftPWM.SetAllHSV(hue, 255, 255); 
}

void randomColors(void){  // Update random LED to random color. Funky!
  unsigned long updateDelay = 100;
  static unsigned long previousUpdateTime;
  if(millis()-previousUpdateTime > updateDelay){
    previousUpdateTime = millis();
    ShiftPWM.SetHSV(random(numRGBLeds),random(360),255,255);
  }
}

void fakeVuMeter(void){ // imitate a VU meter
  static unsigned int peak = 0;
  static unsigned int prevPeak = 0;
  static unsigned long currentLevel = 0;
  static unsigned long fadeStartTime = startTime;
  
  unsigned long fadeTime = (currentLevel*2);// go slower near the top

  unsigned long time = millis()-fadeStartTime;
  currentLevel = time%(fadeTime);

  if(currentLevel==peak){
    // get a new peak value
    prevPeak = peak;
    while(abs(peak-prevPeak)<5){
      peak =  random(numRGBLeds); // pick a new peak value that differs at least 5 from previous peak
    }
  }

  if(millis() - fadeStartTime > fadeTime){
    fadeStartTime = millis();
    if(currentLevel<peak){ //fading in
      currentLevel++;
    }
    else{ //fading out
      currentLevel--;
    }
  }
  // animate to new top
  for(unsigned int led=0;led<numRGBLeds;led++){
    if(led<currentLevel){
      int hue = (numRGBLeds-1-led)*120/numRGBLeds; // From green to red
      ShiftPWM.SetHSV(led,hue,255,255); 
    }
    else if(led==currentLevel){
      int hue = (numRGBLeds-1-led)*120/numRGBLeds; // From green to red
      int value;
      if(currentLevel<peak){ //fading in        
        value = time*255/fadeTime;
      }
      else{ //fading out
        value = 255-time*255/fadeTime;
      }
      ShiftPWM.SetHSV(led,hue,255,value);       
    }
    else{
      ShiftPWM.SetRGB(led,0,0,0);
    }
  }
}

void rgbLedRainbow(unsigned long cycleTime, int rainbowWidth){
  // Displays a rainbow spread over a few LED's (numRGBLeds), which shifts in hue. 
  // The rainbow can be wider then the real number of LED's.
  unsigned long time = millis()-startTime;
  unsigned long colorShift = (360*time/cycleTime)%360; // this color shift is like the hue slider in Photoshop.

  for(unsigned int led=0;led<numRGBLeds;led++){ // loop over all LED's
    int hue = ((led)*360/(rainbowWidth-1)+colorShift)%360; // Set hue from 0 to 360 from first to last led and shift the hue
    ShiftPWM.SetHSV(led, hue, 255, 255); // write the HSV values, with saturation and value at maximum
  }
}

