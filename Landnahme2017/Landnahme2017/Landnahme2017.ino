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

 // Pinbelegungen, einfach umschreiben wenns anders gebraucht ist.
#define RST_PIN 5
#define SS_PIN 53

#define AntikeLed 30
#define ElbenLed 31
#define KometLed 32
#define HDCLed 33
#define ImpLed 34
#define KroneLed 35
#define LichtLed 36
#define NorrelagLed 37
#define OHLLed 38
#define PilgerLed 39
#define ZKLed 40
#define StadtLed 41
#define LesathLed 42
#define ThekiLed 43

#define AmbientLed1 22
#define AmbientLed2 23
#define AmbientLed3 24
#define AmbientLed4 25

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
#define Theki 13

#define Free 1
#define Activated 2
#define Blocked 3

short CurrentPhase;
short CurrentOwner;

unsigned long CurrentTime;
unsigned long CurrentLongestTime;
unsigned long TempTime;

bool HasPrintedStatusBool;

File DataFile;
File TimeData;

void setup() {
  Serial.begin(9600);
  pinMode(10,OUTPUT);
  pinMode(53,OUTPUT);
  digitalWrite(10,HIGH);
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);

  pinMode(AntikeLed,OUTPUT);
  pinMode(ElbenLed,OUTPUT);
  pinMode(KometLed,OUTPUT);
  pinMode(HDCLed,OUTPUT);
  pinMode(ImpLed,OUTPUT);
  pinMode(KroneLed,OUTPUT);
  pinMode(LichtLed,OUTPUT);
  pinMode(NorrelagLed,OUTPUT);
  pinMode(OHLLed,OUTPUT);
  pinMode(PilgerLed,OUTPUT);
  pinMode(ZKLed,OUTPUT);
  pinMode(StadtLed,OUTPUT);
  pinMode(LesathLed,OUTPUT);
  pinMode(ThekiLed,OUTPUT);
  
  Serial.println("Starting up...");  

  for(int i=30;i<=43;i++){
    SetLights(i,1);
  }

  CurrentPhase = 1;
  CurrentOwner = 17;

    if(!SD.begin(4)){
    Serial.println("SD Karte konnte nicht initialisiert werden.");
  }
  else{
    Serial.println("SD Karte initialisiert");
  }

  ReadDataFile();
  PrintCurrentStatus();
  HasPrintedStatusBool = false;

  delay (500);
  watchdogSetup();

  Serial.println("Setup Done");
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
  if(HasPrintedStatusBool==false&&millis()%120000>195000&&millis()%120000<199999)
  {
    PrintCurrentStatus();
    SaveMomentData();
  }
  else if(millis()%60000<55000)
  {
    HasPrintedStatusBool=false;
  }
  delay(1000);
}

void watchdogSetup(void){
 cli();
 wdt_reset();
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

void SetNewOwner(int newOwner){
  CurrentOwner=newOwner;
}

void SetLights(int ledNumber, int phase){
  for(int i=30;i<=43;i++){
    digitalWrite(i,LOW);
  }
  digitalWrite(ledNumber,HIGH);
  delay(500);
}

void PrintCurrentStatus(){
  Serial.println("Statusupdate:");
  Serial.print("Aktueller Status: ");
  Serial.println(CurrentPhase);
  Serial.print("Aktueller Besitzer: ");
  Serial.println(CurrentOwner);
  Serial.print("Zeit seit letztem Start: ");
  Serial.print(millis()/1000 + 1);
  Serial.println(" Sekunden.");
  Serial.print("Gesamte Laufzeit: ");
  Serial.print(CurrentTime);
  Serial.println(" Sekunden.");
  Serial.println();
  HasPrintedStatusBool = true;
}

ISR(WDT_vect)
{
  Serial.println();
  Serial.println("Interrupt");
  Serial.println("------------------------------------------------------------");
}

void UpdateRuntime(){
  TimeData=SD.open("data.txt", FILE_WRITE);
  if(TimeData){
    Serial.println("Write TimeData");
    TimeData.println(millis());
  }else{
    Serial.println("Could not write TimeData");
  }
  TimeData.close();
  delay(200);  
}

void SaveMomentData(){
  DataFile = SD.open("data.txt", FILE_WRITE);
  if(DataFile){
    DataFile.print('t');
    DataFile.println(millis()/1000);
    DataFile.print('o');
    DataFile.println(CurrentOwner);
    DataFile.print('p');
    DataFile.println(CurrentPhase);
    DataFile.close();
  }else{
    Serial.println("Error writing text");
  }
  DataFile.close();
}

void ReadDataFile(){
  CurrentTime=(int)millis()/1000;
  DataFile=SD.open("data.txt");
  int currentLongestTime = 0;
  int tempOwner;
  int tempPhase;
  char inputarray[1000];
  int index=0;
  char readChar;
  if(DataFile){
    while(DataFile.available()){
      readChar=DataFile.read();
      if(readChar!='\n'){
        inputarray[index]=readChar;
        index++;
      }else{
        
        inputarray[index]='\0';
        int tempTime;
        

        if(inputarray[0]=='t'){
          tempTime=atoi(&inputarray[1]);
          if(tempTime>=currentLongestTime){
            currentLongestTime=(int)tempTime;
          }else{
            CurrentTime+=(int)currentLongestTime;
            currentLongestTime=(int)tempTime;
          }
        }else
        if(inputarray[0]=='o'){
          tempOwner=atoi(&inputarray[1]);
        }else
        if(inputarray[0]=='p'){
          tempPhase=atoi(&inputarray[1]);
        }      
        index=0;
      }
    }
    CurrentPhase=tempPhase;
    SetNewOwner(tempOwner);
  }else{
    Serial.println("Could not open data.txt");
  }
}

