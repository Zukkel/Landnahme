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
#include <SD.h>
#include <MFRC522.h>
#include <avr/wdt.h>


#define RST_PIN 5
#define SS_PIN 53

MFRC522 mfrc522(SS_PIN, RST_PIN);

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

#define minuten 60000

#define Free 1
#define Activated 2
#define Blocked 3

//die zeiten für die phasen
const unsigned long tn = 1*minuten;      //Landnahmezeit, Zeit zum Einnehmen
const unsigned long tb = 1*minuten;      //Blockzeit, Zeit nach dem Einnehmen


short CurrentPhase;
short CurrentOwner;

unsigned long Time;
bool HasPrintedStatusBool;

File DataFile;

void setup() {
  Serial.begin(9600);
  pinMode(10,OUTPUT);
  pinMode(53,OUTPUT);
  digitalWrite(10,HIGH);
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);
  Serial.println("Starting up...");

  CurrentPhase = 1;
  CurrentOwner = 17;

    if(!SD.begin(4)){
    Serial.println("SD Karte konnte nicht initialisiert werden.");
  }
  else{
    Serial.println("SD Karte initialisiert");
  }

  PrintCurrentStatus();
  HasPrintedStatusBool = false;

  delay (500);
  watchdogSetup();
}

void loop() {
  wdt_reset();
  switch(CurrentPhase)
  {
    case(Free):

    break;

    case(Activated):


    break;

    case(Blocked):
    

    break;
  }


  if(HasPrintedStatusBool==false&&millis()%60000>55000&&millis()%60000<59999)
  {
    PrintCurrentStatus();
  }
  else if(millis()%60000<55000)
  {
    HasPrintedStatusBool=false;
  }
}

void watchdogSetup(void){
 cli();
 wdt_reset();
/*
 WDTCSR configuration:
 WDIE = 1: Interrupt Enable
 WDE = 1 :Reset Enable
 See table for time-out variations:
 WDP3 = 0 :For 1000ms Time-out
 WDP2 = 1 :For 1000ms Time-out
 WDP1 = 1 :For 1000ms Time-out
 WDP0 = 0 :For 1000ms Time-out
*/
// Enter Watchdog Configuration mode:
WDTCSR |= (1<<WDCE) | (1<<WDE);
// Set Watchdog settings:
 WDTCSR = (1<<WDIE) | (1<<WDE) |
(1<<WDP3) | (0<<WDP2) | (0<<WDP1) |
(1<<WDP0);

sei();
}

short CheckForCard(){
  
}

void SetNewOwner(){
  
}

void SaveMomentData(){
  DataFile = SD.open("data.txt", FILE_WRITE);

  if(DataFile){
    Serial.println("Schreibe Daten");
    PrintCurrentStatus();
    DataFile.print("Aktuelle Zeit: ");
    DataFile.print(millis()/1000);
    DataFile.println(" Sekunden");
    DataFile.close();
    Serial.println("Done");
  }else{
    Serial.println("Error writing text");
  }
}

void LoadLastMomentData(){
  
}

void PrintCurrentStatus(){
  Serial.println("Statusupdate:");
  Serial.print("Aktueller Status: ");
  Serial.println(CurrentPhase);
  Serial.print("Aktueller Besitzer: ");
  Serial.println(CurrentOwner);
  Serial.print("Zeit seit letztem Start: ");
  Time = millis()/60000 + 1;
  Serial.print(Time);
  Serial.println(" Minuten.");
  Serial.println();
  HasPrintedStatusBool = true;
}


