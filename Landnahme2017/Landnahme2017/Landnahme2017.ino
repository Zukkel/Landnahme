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
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */


#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <avr/wdt.h>

 // Alle Einstellungen werden hier oben vorgenommen, nicht im Code!
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

#define AmbientLed 3

#define AntikeId "Test"
#define ElbenId "test2"
#define KometId "test3"
#define HDCId "b3df6b00"
#define ImperiumId "94134bea"
#define KroneId "test6"
#define LichtId "test7"
#define NorrelagId "d39c8900"
#define OHLId "73836900"
#define PilgerId "c5df9d2c"
#define ZKId "test11"
#define StadtId "test12"
#define LesathId "test13"
#define ThekiId "test14"

const unsigned int TimeToBlock=60*1;
const unsigned int Active=10*1;

//Ende der Einstellungen

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

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
short PreviousOwner;

unsigned long CurrentTime;
unsigned long CurrentLongestTime;
unsigned long TempTime;
unsigned long BlockTime;
unsigned long ActivationTime;
unsigned long failCount;

unsigned long LedStatus;
unsigned long LedIncrement;

bool HasPrintedStatusBool;

File DataFile;
File TimeData;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
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

  pinMode(AmbientLed,OUTPUT);
  
  
  Serial.println("Starting up...");

  digitalWrite(AmbientLed,HIGH);
  
  for(int i=30;i<=43;i++){
    SetLights(i);
  } 

  for(int i=0;i<2;i++){   
  
    for(int k=0;k<255;k++){
      analogWrite(AmbientLed,k);
      delay(5);
    }
    for(int k=255;k>0;k--){
      analogWrite(AmbientLed,k);
      delay(5);
    }
  }
  // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

  CurrentPhase = 1;
  CurrentOwner = 17;
  BlockTime=0;
  ActivationTime=0;
  failCount=0;
  LedStatus=1;
  LedIncrement=1;

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
  int check;
  delay(3);
  
  switch(CurrentPhase)
  {
    case(Free):
    if(LedStatus>254||LedStatus<1){
      LedIncrement=-LedIncrement;
    }
    LedStatus+=LedIncrement;
    analogWrite(AmbientLed,LedStatus);
    
    check = CheckForCard();
    if(check!=17&&check!=CurrentOwner){
      ActivateShrine(check);
    }
    break;

    case(Activated):
        
    if(LedStatus>254){
      LedStatus=0;    
    }
    LedStatus+=3;
    analogWrite(AmbientLed,LedStatus);
    if(ActivationTime+Active>=CurrentTime+millis()/1000){
      check=CheckForCard();
      if(check==17){
        failCount++;
        if(failCount>50){
          FreeShrine();
          CurrentOwner=PreviousOwner;
          SetLights(PreviousOwner);
          failCount=0;
        }        
      }else{
        failCount=0;
      }      
    }else{
      while(check!=CurrentOwner){
        check=CheckForCard();
      }
      BlockShrine(check);
    }
    break;

    case(Blocked):
    check=CheckForCard();
    if(check!=17){
      for(int k=0;k<3;k++){      
        for(int i=0;i<=255;i++){
          analogWrite(AmbientLed,i);
          delay(3);
        }
      }
      analogWrite(AmbientLed,0);
      delay(2000);
    }
    if(BlockTime+TimeToBlock<=CurrentTime+millis()/1000){
      FreeShrine();
    }
    break;
  }
  if(HasPrintedStatusBool==false&&millis()%120000>115000&&millis()%120000<119999)
  {
    PrintCurrentStatus();
    SaveMomentData();
  }
  else if(millis()%120000<115000)
  {
    HasPrintedStatusBool=false;
  }
}

void BlockShrine(int Blocker){
  CurrentPhase=3;
  analogWrite(AmbientLed,0);
  Serial.println("Shrine blocked");
  ActivationTime=0;
  CurrentOwner=Blocker;
  SetLights(Blocker);
  BlockTime=CurrentTime+millis()/1000;
  PrintCurrentStatus();
  SaveMomentData();
  delay(10);
}

void ActivateShrine(int Activator){
  CurrentPhase=2;
  Serial.println("Shrine activated");
  analogWrite(AmbientLed,100);
  PreviousOwner=CurrentOwner;
  CurrentOwner=Activator;
  SetLights(Activator);
  ActivationTime=CurrentTime+millis()/1000;
  PrintCurrentStatus();
  SaveMomentData();
  delay(10);
}

void FreeShrine(){
  CurrentPhase=1;
  Serial.println("Shrine freed");
  analogWrite(AmbientLed,100);
  ActivationTime=0;
  BlockTime=0;
  PrintCurrentStatus();
  SaveMomentData();
  delay(10);
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

int CheckForCard(){
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()){
    return 17;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()){
    return 17;
  }
  String rfidUid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    rfidUid += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println(rfidUid);
  if(rfidUid==AntikeId){
    return Antike ;
  }else if(rfidUid==ElbenId ){
    return Elben ;
  }else if(rfidUid==KometId ){
    return Komet ;
  }else if(rfidUid==HDCId){
    return HDC;
  }else if(rfidUid==ImperiumId ){
    return Imperium ;
  }else if(rfidUid==KroneId ){
    return Krone ;
  }else if(rfidUid==LichtId ){
    return Licht ;
  }else if(rfidUid==NorrelagId ){
    return Norrelag ;
  }else if(rfidUid==OHLId ){
    return OHL ;
  }else if(rfidUid==PilgerId ){
    return Pilger ;
  }else if(rfidUid==ZKId ){
    return Zusammenkunft ;
  }else if(rfidUid==StadtId ){
    return Stadt ;
  }else if(rfidUid==LesathId ){
    return Lesath ;
  }else if(rfidUid==ThekiId ){
    return Theki ;
  }else{
    return 17;
  }
}

void SetLights(int ledNumber){
  for(int i=30;i<=43;i++){
    digitalWrite(i,LOW);
  }

  ledNumber+=30;
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
  Serial.print(millis()/1000);
  Serial.println(" Sekunden.");
  Serial.print("Gesamte Laufzeit: ");
  Serial.print(CurrentTime+millis()/1000);
  Serial.println(" Sekunden.");
  Serial.print("Blocktime: ");
  Serial.print(BlockTime);
  Serial.println(" Sekunden.");
  Serial.print("ActivationTime: ");
  Serial.print(ActivationTime);
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

void SaveMomentData(){
  DataFile = SD.open("data.txt", FILE_WRITE);
  if(DataFile){
    DataFile.print('t');
    DataFile.println(millis()/1000);
    DataFile.print('o');
    DataFile.println(CurrentOwner);
    DataFile.print('p');
    DataFile.println(CurrentPhase);
    DataFile.print('b');
    DataFile.println(BlockTime);
    DataFile.print('a');
    DataFile.println(ActivationTime);
    DataFile.close();
  }else{
    Serial.println("Error writing text");
  }
  DataFile.close();
}

void ReadDataFile(){
  CurrentTime=0;
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
        }else
        if(inputarray[0]=='b'){
          BlockTime=atoi(&inputarray[1]);
        }else
        if(inputarray[0]=='a'){
          ActivationTime=atoi(&inputarray[1]);
        }
        index=0;
      }
    }
    CurrentTime+=(int)currentLongestTime+(int)millis()/1000;
    CurrentPhase=tempPhase;
    CurrentOwner=tempOwner;
    SetLights(CurrentOwner);
  }else{
    Serial.println("Could not open data.txt");
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void CrossFade(){
  
}

