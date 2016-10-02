#include <Time.h>
#include <SD.h>
#include <SPI.h>

/*
Dust Sensor Data Logging

SD CARD:
MISO - pin 50
MOSI - pin 51 
SCK  - pin 52
CS   - pin 45

DUST SENSOR :
P1 (<= 1μm) - pin 8

*/
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by Unix time_t as ten ASCII digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

int SD_MISO  = 50;
int SD_MOSI  = 51;
int SD_SCK   = 52;
int SD_CS    = 45;

int GROVE_P1 = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s&nbsp;;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

char requestSyncChar = 'T';
void setup(){
  Serial.begin(9600);
  Serial.println("Initializing SD Card...");
  if(!SD.begin(SD_CS)){
   Serial.println("\tInitialization Failed"); 
  }
  Serial.println("\tInitialization complete");
  Serial.println("\nInitializing timer");
  Serial.println("\twaiting for sync message...");
  while(timeStatus() == timeNotSet){
    Serial.println("T");
    delay(1000);
    if(Serial.available() ) 
    {
      processSyncMessage();
    }
  }
  Serial.println("\tsync complete.");
}

void loop(){
  digitalClockDisplay();  
  delay(1000);
}
void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
void processSyncMessage() {
  // if time sync available from serial port, update time and return true
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of header & 10 ASCII digits
    char c = Serial.read() ; 
    Serial.print(c);  
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number    
        }
      }   
      setTime(pctime);   // Sync Arduino clock to the time received on the serial port
    }  
  }
}
