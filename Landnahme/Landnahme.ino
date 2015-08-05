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
#define rgb_red_PIN 3
#define servoLED1_PIN 4     //oben
#define rgb_green_PIN 5
#define rgb_blue_PIN 6
#define servoLED2_PIN 7     //unten
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
const unsigned long ta = 1*minuten;      //Aktivierungszeit, Zeit in Phase 1
const unsigned long tl = 1*minuten;      //Ladezeit, Zeit in Phase 2
const unsigned long tn = 1*minuten;      //Landnahmezeit, Zeit in Phase 3
const unsigned long tb = 1*minuten;      //Blockzeit, Zeit in Phase 4
                                          //Phase 0 hat keine Zeit und kann beliebig lange gehen

                                        

short _status;
short newKeeper;
short mode;
long servoValue;
short servoDirection;

short ledValue;
short ledDirection;
short ledStatus;

unsigned long activeTime;

int failCount;

int scan;

void setup() {
Serial.begin(9600);
  pinMode(servoLED1_PIN, OUTPUT);
  pinMode(servoLED2_PIN,OUTPUT);
  pinMode(rgb_red_PIN,OUTPUT);
  pinMode(rgb_blue_PIN,OUTPUT);
  pinMode(rgb_green_PIN,OUTPUT);


  _status=Free;
  newKeeper=Neutral;
  mode = real;
  
  servoDirection=1;
  ledDirection=1;
  ledStatus=0;
  ledValue=255;

  failCount=0;

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
      if((millis()-activeTime)<ta)
      {
        if(!mfrc522.PICC_IsNewCardPresent())
        {
          failCount++;
          if(failCount>25)
          {
            _status--;
            delay(10);
            setShrine();
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
        delay(10);
        failCount=0;
        Serial.println(millis()-activeTime);
        activeTime=millis();
        setShrine();
        if(!myServo.attached())
          myServo.attach(servo_PIN);
        Serial.println("wechsel zu loading");
        
      }
      break;
 
      case(loading):
      pulse();
        if((millis()-activeTime)<tl)
        {

        }
        else
        {
            _status++;
            delay(10);
            activeTime=millis();
            setShrine();
            Serial.println("wechsel zu active");        
        }

      break;

      case(active):
      pulse();
      if((millis()-activeTime)<tn)
      {
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          if(myServo.attached())
            myServo.detach();
          delay(3000);
          newKeeper = scan;
          _status=0;
          delay(10);
          setShrine();  
        }
        else if((scan!=Unbekannt)&&(scan!=Demo)&&(scan!=Diagnostic))
        {
          if(myServo.attached())
            myServo.detach();
          delay(3000);
          newKeeper=scan;
          _status++;
          delay(10);
          setShrine();
          activeTime=millis();
          Serial.println("wechsel zu blocked");          
        }
        
      }
      else
      {
        if(myServo.attached())
          myServo.detach();
        delay(3000);
        newKeeper=Neutral;
        _status=0;
        delay(10);
        setShrine();
        activeTime=millis();
        Serial.println("zurück zu free, weil keine karte vorhanden");
      }
      
      break;

      case(blocked):
      if((millis()-activeTime)<tb)
      {
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          newKeeper = scan;
          _status=0;
          delay(10);
          activeTime=millis();
          setShrine();  
        }
      }
      else
      {
        _status=0;
        delay(10);
        activeTime=millis();
        setShrine();
        Serial.println("wechsel zu free");
      }
      break;

      default:
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          newKeeper = scan;
          setShrine();
        }
        else if(scan==Demo)
        {
          mode=demo;
        }
        else if(scan==Diagnostic)
        {
          mode=diagnostic;
        }
        else if(scan==newKeeper)
        {
          
          pulse();

        }
        else if(scan!=Unbekannt)
        {
          _status++;
          delay(10);
          failCount=0;
          activeTime=millis();
          setShrine();
          Serial.println("wechsel zu activating");
        }
        else
        {
          digitalWrite(5,LOW);
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
      delay(10);
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
   {
      return Unbekannt;
   }
   else if (!mfrc522.PICC_ReadCardSerial())
   {
    Serial.println(" geht nicht");
    return Unbekannt;                                                            //Unbekannt bedeutet das keine karte gefunden wurde = 17
   }
   else
   {
    String rfidUid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      rfidUid += String(mfrc522.uid.uidByte[i], HEX);
    }

    Serial.println(rfidUid);
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
    if(rfidUid == "b3afcb02")
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
    //if(rfidUid == "0006540c50")
    if(rfidUid == "d5afa72c")
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
    return Unbekannt;     
  }
}

void setServo()
{
  myServo.write(7.5+15*newKeeper);
  Serial.println("Servo moved");
  delay(3000);
}

void setShrine()
{

  switch(_status)
  {
    Serial.println(_status);
    case(activating):
    //RGB auf Blau
    Serial.println(_status);
    digitalWrite(rgb_red_PIN, HIGH);
    digitalWrite(rgb_green_PIN, HIGH);
    digitalWrite(rgb_blue_PIN, LOW);
    break;
    
    case(loading):
    digitalWrite(rgb_green_PIN, HIGH);
    digitalWrite(rgb_blue_PIN, HIGH);
    
    //servo leds an
    digitalWrite(servoLED1_PIN, HIGH);    
    digitalWrite(servoLED2_PIN, LOW);    
    break;

    case(active):
    digitalWrite(rgb_red_PIN, HIGH);
    digitalWrite(rgb_green_PIN, HIGH);
    digitalWrite(rgb_blue_PIN, HIGH);
    break;

    case(blocked):
    //servo auf Neues Lager setzen
    if(!myServo.attached())
      myServo.attach(servo_PIN);
    setServo();
    myServo.detach();
        
    //RGB auf Rot setzen
    digitalWrite(rgb_red_PIN, LOW);
    digitalWrite(rgb_green_PIN, HIGH);
    digitalWrite(rgb_blue_PIN, HIGH);

    //Servo-LEDs an, SL-LEDs aus
    digitalWrite(servoLED1_PIN, HIGH);
    digitalWrite(servoLED2_PIN, LOW);  
    break;

    default:
    Serial.println(newKeeper);
    //RGB auf Grün setzen
    digitalWrite(rgb_green_PIN, LOW);
    digitalWrite(rgb_red_PIN, HIGH);
    digitalWrite(rgb_blue_PIN, HIGH);
    
    if(newKeeper==Lesath||newKeeper==Neutral||newKeeper==Dunkel)
    {
      digitalWrite(servoLED1_PIN, LOW);
      digitalWrite(servoLED2_PIN, HIGH);
      //Servo auf SL Lager setzen
      if(!myServo.attached())
        myServo.attach(servo_PIN);
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
      delay(3000);
      myServo.detach();
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
    delay(30);
      myServo.write(servoValue);
      delay(5);
      servoValue+=servoDirection;
      if(servoValue%180==0)
        servoDirection*=-1;

      analogWrite(3,ledValue);
      ledValue+=ledDirection;
      if((ledValue%255)==0)
        ledDirection*=-1;

    break;

    case(active):
    delay(30);
      myServo.write(servoValue);
      delay(5);
      servoValue+=servoDirection;
      if(servoValue%180==0)
        servoDirection*=-1;

      switch(ledStatus)
      {
        case(0):
        analogWrite(3,ledValue);
        analogWrite(5,255-ledValue);
        ledValue++;
        if(ledValue>254)
        {
          ledValue=0;
          ledStatus=1;
        }
        break;
      
        case(1):
        analogWrite(6,255-ledValue);
        analogWrite(5,ledValue);
        ledValue++;
        if(ledValue>254)
        {
          ledValue=0;
          ledStatus=2;
        }
        break;
      
        case(2):
        analogWrite(6,ledValue);
        analogWrite(3,255-ledValue);
        ledValue++;
        if(ledValue>254)
        {
          ledValue=0;
          ledStatus=0;
      
        }
        break;
      }

      case(Free):
      delay(10);
      analogWrite(5,ledValue);
      ledValue+=ledDirection;
      if((ledValue%255)==0)
        ledDirection*=-1;
      break;
  }
}


