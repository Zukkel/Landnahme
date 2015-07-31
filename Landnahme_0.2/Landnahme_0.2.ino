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

#define real 0
#define demo 1
#define diagnostic 2

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
#define Demo 15
#define Unbekannt 16


const unsigned long ta = 10*minuten;      //Aktivierungszeit, Zeit in Phase 1
const unsigned long tl = 60*minuten;      //Ladezeit, Zeit in Phase 2
const unsigned long tn = 20*minuten;      //Landnahmezeit, Zeit in Phase 3
const unsigned long tb = 180*minuten;      //Blockzeit, Zeit in Phase 4
                                          //Phase 0 hat keine Zeit und kann beliebig lange gehen

short _status;
short lastKeeper;
short newKeeper;
short mode;
short servoValue;

unsigned long activeTime;

int failCount;

int scan;




void setup() {
Serial.begin(9600);
  myServo.attach(3);

  _status=Free;

  newKeeper=Neutral;

  servoValue = 90;                             //normal 90
  myServo.write(servoValue);


  failCount=0;
  mode = 0;

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
    switch(_status)
    {
      case(activating):
      
      break;
      
      case(loading):

      break;

      case(active):
      
      break;

      case(blocked):

      break;

      default:
      
      return;

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
        failCount=0;
        activeTime=micros();
        setShrine();
      }
      break;
 
      case(loading):
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
          setServo(newKeeper);
          setShrine();
          activeTime=micros();          
        }
        
      }
      else
      {
        _status=0;
        setServo(newKeeper);
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
        else if(scan!=Unbekannt)
        {
          _status++;
          failCount=0;
          activeTime=micros();
          setShrine();
        }
      
      return;

    }
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
  }
}

void setServo(int Lager)
{

  return;
}

void setShrine()
{

  return;
}

