#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>

RTC_DS1307 RTC;
DateTime now;
char print_date[12];
char print_time[8];

//SD Card
byte SD_MISO = 50;
byte SD_MOSI = 51;
byte SD_SCK  = 52;
byte SD_CS   = 45;

File logFile;


//DUST
byte GROVE_P1 = 8;

unsigned long duration;
unsigned long start_time;
unsigned long sample_time_ms = 30000;
unsigned long LPO = 0;
float ratio = 0;
float concentration = 0;

char dust_output[16]; 


LiquidCrystal lcd(31, 30, 29, 28, 27, 26);
char waiting_arr[6] = {223, 165, 46 , 161, 111, 235};
byte waiting_ptr = 0;
byte waiting_len = 5;
boolean top = true;

void setup(){
  Serial.begin(9600);
  
  //wait a few secs to give ths lcd some quiet time
  
  delay(1500);
  
  /*
    Initialize the LCD
  */
  
  lcd.begin(16, 2);
  lLog("LCD Initialized", 0);  
  delay(500);
  
  /*
    Initialize the SD Card, and create the log file if it doesn't exist
  */
  
  pinMode(53, OUTPUT);
  if( !SD.begin(SD_CS) ){
    lLog( "SD Card", 0 );
    lLog( "Failed", 1 );
    return;
  }else if( !SD.exists("log.csv") ){
    logFile = SD.open("log.csv", FILE_WRITE);
    logFile.println( "date, time, pcs/0.01cf" );
    logFile.close();
  }
  lLog("SD Initialized", 0);
  delay(500);
  
  /*
    Configure RTC if necessary
  */
  
  Wire.begin();
  RTC.begin();
  if( !RTC.isrunning() ){
    lLog( "Setting RTC...", 0 );
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }else{
    lLog("Resuming RTC", 0);
  }
  delay(500);
  
  /*
    Set that Dust Sensor up, bro
  */
  
  pinMode(GROVE_P1, INPUT);
  start_time = millis();
  lcd.clear();
  
  /*
    Good to go
  */
  
  lLog("Waiting on", 0);
  lLog("the air, d00d", 1);
}

/*
  just another a log wrapper
*/
void lLog( char* msg, byte row ){
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(msg);
}

void lLog( float msg, byte row ){
   lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(msg);
}

/*
  various sprintf functions
*/
void get_date(){
  sprintf(print_date, "%02d-%02d-%04d", now.month(), now.day(), now.year());
}

void get_time(){
  sprintf(print_time, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
}

void write_file(float c){
  
  logFile = SD.open("log.csv", FILE_WRITE);
  if(logFile){
    now = RTC.now();
    get_date();
    logFile.print(print_date);
    logFile.print(",");
    get_time();
    logFile.print(print_time);
    logFile.print(",");
    logFile.println(c);
    logFile.close();
  }else{
    lLog("SD Card", 0);
    lLog("File Error :(", 1);
  }
  
}

/*
  To indicate that something is happening
  TODO: set this up on an independant timer for smoothness
*/

void waiting(){
  if(top){
    lcd.setCursor(15, 0);
  }else{
    lcd.setCursor(15, 1);
  }
  
  lcd.print( waiting_arr[waiting_ptr] );
  
  waiting_ptr++;
  if(waiting_ptr > waiting_len){
    waiting_ptr = 0;
    top = !top;
  }
}

void loop(){
  waiting();
  duration = pulseIn(GROVE_P1, LOW);
  LPO = LPO+duration;
   if ((millis()-start_time) >= sample_time_ms)//if the sampel time = = 30s
    {
      ratio = LPO/(sample_time_ms*10.0);  // Integer percentage 0=&gt;100
      concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
      lLog(concentration, 0);
      lLog("pcs/0.01cf", 1);
      LPO = 0;
      start_time = millis();
      write_file(concentration);
    }
}
