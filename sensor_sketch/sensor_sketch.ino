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

//Dust Sensor
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s&nbsp;;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

//SD Card
File sessionFile;
const byte maxFileNameLen = 12;
char sessionFilename[maxFileNameLen];
char logFileExt[] = ".csv";

//time sync
char requestSyncChar = 'T';

void setup(){
  Serial.begin(9600);
  pinMode(53, OUTPUT);
  Serial.println("Initializing SD Card...");
  if(!SD.begin(SD_CS)){
    Serial.println("\tInitialization Failed"); 
    return;
  }
  Serial.println("\tInitialization complete");
  Serial.println("\nInitializing timer");
  Serial.println("\twaiting for sync message...");
  Serial.println("T");
  while(timeStatus() == timeNotSet){
    delay(1000);
    if(Serial.available() ) 
    {
      processSyncMessage();
    }
  }
  Serial.println("\tsync complete.");
  printFreeRam();
  createFile();

}
char timeTest[12];
void loop(){
  concentration = random();
  Serial.println("Pushing some log data...");
  Serial.println(concentration);
  pushLogData(concentration);
  delay(2000);
}
void printFreeRam(){
  Serial.print("Free RAM\t:\t");
  Serial.println(FreeRam());
}
  
void createFile(){
  getFilename(sessionFilename);
  Serial.print("Checking if ");
  Serial.print(sessionFilename);
  Serial.print(" exists...");
  printFreeRam();
  if(!SD.exists(sessionFilename) ){
    Serial.print("File doesn't exist.\n\tCreating ");
    Serial.print(sessionFilename);
    Serial.print("...\n");
    printFreeRam();
    sessionFile = SD.open(sessionFilename, FILE_WRITE);
    sessionFile.rewindDirectory();
    if(sessionFile){
      sessionFile.println("Date, Time, pcs/0.01cf");
      sessionFile.close();
      Serial.println("\tFile created.");
    }else{
      Serial.println("\tError opening file.");
      
    }
  }else{
    Serial.println("File Exists..."); 
  }
}
void pushLogData(long concentration){
  char date[8];
  char time[8];
  dateFormat(date, "-");
  timeFormat(time);
  sessionFile = SD.open(sessionFilename, FILE_WRITE);
  if(sessionFile){
    sessionFile.print(date);
    sessionFile.print(",");
    sessionFile.print(time);
    sessionFile.print(",");
    sessionFile.print(concentration);
    sessionFile.print("\n");
  }else{
    Serial.println("\tLog update failed.");
  }
}
void getFilename(char __fn[]){
  char name[8];
  dateFormat(name, ""); 
  strcpy(__fn, name);
  strcat(__fn, logFileExt);
}
void dateFormat(char __date[], char delimiter[]){
  char _month[3];
  char _day[3];
  char _year[5];

  itoa(month(), _month, 10);
  itoa(day(), _day, 10);
  itoa(year(), _year, 10);

  strncpy(_year, &_year[2], 5);
  padString(_month);
  padString(_year);

  strcpy(__date, _month);
  strcat(__date, delimiter);
  strcat(__date, _day);
  strcat(__date, delimiter);
  strcat(__date, _year);
}
void timeFormat(char __time[]){
  char _hour[3];
  char _minute[3];
  char _second[5];

  itoa(hour(), _hour, 10);
  itoa(minute(), _minute, 10);
  itoa(second(), _second, 10);

  padString(_hour);
  padString(_minute);
  padString(_second);

  strcpy(__time, _hour);
  strcat(__time, ":");
  strcat(__time, _minute);
  strcat(__time, ":");
  strcat(__time, _second);
}
void padString(char __STR[]){
  char __TMP[2];
  if(strlen(__STR) < 2){
    strcpy(__TMP, __STR);
    strcpy(__STR, "0");
    strcat(__STR, __TMP);
  } 
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

