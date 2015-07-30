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

#define minuten 60000

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


const unsigned long ta = 10*minuten;      //Aktivierungszeit, Zeit in Phase 1
const unsigned long tl = 10*minuten;      //Ladezeit, Zeit in Phase 2
const unsigned long tn = 20*minuten;      //Landnahmezeit, Zeit in Phase 3
const unsigned long tb = 30*minuten;      //Blockzeit, Zeit in Phase 4
                                          //Phase 0 hat keine Zeit und kann beliebig lange gehen

short _status;
short lastKeeper;
short newKeeper;

short servoValue;

unsigned long activeTime;

int failCount;

int scan;



void setup() {
Serial.begin(9600);
  myServo.attach(3);

  _status=Free;
  lastKeeper=Neutral;
  newKeeper=Neutral;

  servoValue = 90;                             //normal 90
  myServo.write(servoValue);


  failCount=0;

  activeTime=0;
  _status=0;
  scan = 0;

  SPI.begin();
  mfrc522.PCD_Init();


  Serial.println("We are ready to start");

}

void loop() {
  if(micros()<3*minuten)                                //eine Testphase am anfang. Dauert 3 minuten
  {
    if(micros()<1000*30)
    {
      _status = 0;
    }
    else if(micros()<1000*90)
    {
      _status = 1;
    }
    else if(micros()<1000*120)
    {
      _status = 2;
    }
    else if(micros()<1000*180)
    {
      _status = 3;
    }
    else if(micros()<1000*290)
    {
      _status = 4;
    }
    else
    {
      _status = 0;
    }

  }
  else
  {
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
        activeTime=micros();
      }
      break;
      
      case(loading):

      break;

      case(active):
      
      break;

      case(blocked):

      break;

      default:
        scan=scanForCard();
        if(scan==0||scan==7||scan==14)
        {
          lastKeeper = scan;
          newKeeper = scan;

          myServo.write(12*scan);
          delay(15);
          myServo.write(myServo.read());
        }
        else if(scan!=15)
        {
          _status++;
          failCount=0;
          activeTime=micros();
        }
      
      return;

    }
  }
  

}

int scanForCard()
{
   if (!mfrc522.PICC_IsNewCardPresent())
    return 15;                                                            //15 bedeutet das keine karte gefunden wurde
   else
   {
    String rfidUid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      rfidUid += String(mfrc522.uid.uidByte[i], HEX);
    }

    if(rfidUid == "")
      return 0;
    if(rfidUid == "")
      return 1;
    if(rfidUid == "")
      return 2;
    if(rfidUid == "")
      return 3;
    if(rfidUid == "")
      return 4;
    if(rfidUid == "")
      return 5;
    if(rfidUid == "")
      return 6;
    if(rfidUid == "")
      return 7;
    if(rfidUid == "")
      return 8;
    if(rfidUid == "")
      return 9;
    if(rfidUid == "")
      return 10;
    if(rfidUid == "")
      return 11;
    if(rfidUid == "")
      return 12;
    if(rfidUid == "")
      return 13;
    if(rfidUid == "")
      return 14;
  }
}

