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


/*
 * Pin Belegung:
 * Servomotor: 3
 * Ambiente-LED: 4
 * Lade-LED: 5 
 * Ready-LED: 6
 *  
 *  Einer der Reader Pins wird nicht verwendet
 */


/* hier bitte die zeiten der einzelnen Phasen definieren, in milisekunden */


#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define ServoPin 3
#define AmbientPin 4
#define LadePin 5
#define ReadyPin 6


#define Free 0
#define activating 1
#define loading 2
#define active 3
#define blocked 4

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);

Servo myServo;

#define Lesath 0
#define Antike 1
#define Elben 2
#define Komet 3
#define HDC 4
#define Imperium 5
#define Krone 6
#define Neutral 7
#define Licht 8
#define Norrelag 9
#define OHL 10
#define Pilger 11
#define Zusammenkunft 12
#define Stadt 13
#define Dunkel 14


const unsigned long ta = 2*1000;
const unsigned long tl = 10*1000;
const unsigned long tn = 20*1000;
const unsigned long tb = 30*1000;

short Status;
int LastKeeper;
int NewKeeper;
int AmbientValue;
int LadeValue;
int ReadyValue;

int servoValue;

long activeTime;

int A,L,R;

int failCount;

void setup() {
  Serial.begin(9600);
  myServo.attach(3);

  Status=Free;
  LastKeeper=Neutral;
  NewKeeper=Neutral;

  servoValue = 90;                             //normal 90
  myServo.write(servoValue);

  A=1;
  L=1;
  R=1;

  AmbientValue = 192;
  LadeValue = 64;
  ReadyValue = 0;

  failCount=0;

  activeTime=0;

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(AmbientPin, OUTPUT);
  pinMode(LadePin, OUTPUT);
  pinMode(ReadyPin, OUTPUT);

  analogWrite(AmbientPin, AmbientValue);
  analogWrite(LadePin, LadeValue);

  Serial.println("We are ready to start");
  }

void loop() {
  switch(Status)
  {
    case(activating):
      analogWrite(LadePin,LadeValue);
      LadeValue+=L;
      if(LadeValue==255||LadeValue==0)
        L*=-1;
      delay(6);
      activeTime+=6;
      if(!mfrc522.PICC_IsNewCardPresent())
      {
        failCount++;
        if(failCount>10)
        {
          failCount++;
          Status--;
          AmbientValue = 192;
          LadeValue = 64;
          ReadyValue = 0;
        }
        return;
          
        
      }
      else if(activeTime>ta)
      {
        Status++;
        activeTime=0;
             
      }
      failCount=0;
      
      break;

      
    case(loading):
      delay(6);
      activeTime+=6;

      if(servoValue< 90)
       {
        servoValue++;
        myServo.write(servoValue);
       }
       else if(servoValue > 90)
       {
        servoValue--;
        myServo.write(servoValue);
       }

      if(activeTime < 0.75*tl)
      {
        analogWrite(LadePin,LadeValue);
        LadeValue+=L*6;
        if(LadeValue==255||LadeValue==0)
          L*=-1;
      }
      else if(activeTime < 0.90*tl)
      {
        LadeValue=0;
        analogWrite(LadePin,LadeValue);
        analogWrite(ReadyPin,ReadyValue);
        ReadyValue+=R;
        if(ReadyValue==255||ReadyValue==0)
          R*=-1;
      }
      else if(activeTime < tl)
      {
        analogWrite(ReadyPin,ReadyValue);
        ReadyValue+=R*6;
        if(ReadyValue==255||ReadyValue==0)
          R*=-1;
      }
      else
      {
        Status++;
        ReadyValue=255;
        activeTime=0;
        analogWrite(ReadyPin,ReadyValue);
      }
      
      break;

      
    case(active):
      delay(20);
      activeTime+=20;
      if(activeTime > tn)
      {
        Status=0;
        

        AmbientValue = 192;
        LadeValue = 64;
        ReadyValue = 0;

        activeTime=0;
      }
      else
      {

        if ( ! mfrc522.PICC_IsNewCardPresent()) {

            return;
        }
        
        if ( ! mfrc522.PICC_ReadCardSerial()) {

            return;
        }

        String rfidUid = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
          rfidUid += String(mfrc522.uid.uidByte[i], HEX);

        if(rfidUid=="fcb579d5")
        {
            NewKeeper=OHL;
            LastKeeper=OHL;
            Status++;
            activeTime = 0;
            return;
            break;
        }
        
    }
      }
      break;

      
    case(blocked):
      if(servoValue< 12*NewKeeper)
       {
        Serial.println("Stelle Motor ein");
        Serial.println(NewKeeper);
        servoValue++;
        myServo.write(servoValue);
       }
       else if(servoValue > 12*NewKeeper)
       {
        servoValue--;
        myServo.write(servoValue);
       }
       delay(20);
       activeTime+=20;
       Serial.println(activeTime);
       

       
       LadeValue+=L;
       if(LadeValue==0 || LadeValue==255)
       L*=-1;
       analogWrite(LadePin, LadeValue);


       
       if(activeTime>tb)
       {
          Status=0;
          
       }
      break;


    default:               //this is "free"
      analogWrite(AmbientPin, AmbientValue);
      analogWrite(LadePin, LadeValue);
      analogWrite(ReadyPin, ReadyValue);

      AmbientValue+=A;
      LadeValue+=L;
      ReadyValue+=R;

      if(AmbientValue==127 || AmbientValue==255)
        A*=-1;
      if(LadeValue==0 || LadeValue==255)
        L*=-1;
      if(ReadyValue==0 || ReadyValue==255)
        R*=-1;

      delay(20);

       if ( mfrc522.PICC_IsNewCardPresent()) {
            String rfidUid = "";
            for (byte i = 0; i < mfrc522.uid.size; i++) {
              rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
              rfidUid += String(mfrc522.uid.uidByte[i], HEX);
            }
            if(rfidUid=="fcb579d5"&&LastKeeper==10)
              return;

            Status++;
            AmbientValue=255;
            analogWrite(AmbientPin,AmbientValue);
            ReadyValue=0;
            analogWrite(ReadyPin,ReadyValue);
            LadeValue=0;

            activeTime=0;

            }
             
      break;
  }
  

}
