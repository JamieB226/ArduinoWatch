#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#include <Wire.h>
#include <avr/pgmspace.h>
#include "Time.h"

#define ADAFRUITBLE_REQ 5
#define ADAFRUITBLE_RDY 2     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 9
Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);
aci_evt_opcode_t laststatus;

// pin definitions
#define OLED_DATA 11
#define OLED_CLK 13
#define OLED_DC 6
#define OLED_CS 7
#define OLED_RST 8

#define VIBRATION 4
#define BATTSTAT1 A0
#define BATTSTAT2 A1

#include <SSD1306ASCII.h>
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

uint8_t xpos=0,ypos=0;

static unsigned char __attribute__ ((progmem)) GMail_Logo[]={0xFF,0xFF,0x06,0x06,0x0C,0x0C,0x06,0x06,0xFF,0xFF};
static unsigned char __attribute__ ((progmem)) EMail_Logo[]={0xFF,0x83,0x85,0x89,0x91,0x91,0x89,0x85,0x83,0xFF};
static unsigned char __attribute__ ((progmem)) RSS_Logo[]={0xDB,0xDB,0x1B,0xF3,0xEE,0x0E,0xFC,0xF0};
static unsigned char __attribute__ ((progmem)) NestHome_Logo[]={0x00,0x3C,0x7E,0x81,0xE7,0xE7,0xE7,0x81,0x7E,0x3C};
static unsigned char __attribute__ ((progmem)) NestAway_Logo[]={0x00,0x3C,0x7E,0x83,0xED,0xED,0xED,0x83,0x7E,0x3C};
static unsigned char __attribute__ ((progmem)) Charged_Logo[]={0x18,0x38,0x70,0xE0,0x70,0x38,0x1C,0x0E,0x07,0x03};
static unsigned char __attribute__ ((progmem)) Charging_Logo[]={0x00,0x1C,0x3C,0x3F,0xFC,0xFC,0x3F,0x3C,0x1C,0x00};
  
static unsigned char __attribute__ ((progmem)) font[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, // SPACE
  0x00, 0x00, 0x5F, 0x00, 0x00, // !
  0x00, 0x03, 0x00, 0x03, 0x00, // "
  0x14, 0x3E, 0x14, 0x3E, 0x14, // #
  0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
  0x43, 0x33, 0x08, 0x66, 0x61, // %
  0x36, 0x49, 0x55, 0x22, 0x50, // &
  0x00, 0x05, 0x03, 0x00, 0x00, // '
  0x00, 0x1C, 0x22, 0x41, 0x00, // (
  0x00, 0x41, 0x22, 0x1C, 0x00, // )
  0x14, 0x08, 0x3E, 0x08, 0x14, // *
  0x08, 0x08, 0x3E, 0x08, 0x08, // +
  0x00, 0x50, 0x30, 0x00, 0x00, // ,
  0x08, 0x08, 0x08, 0x08, 0x08, // -
  0x00, 0x60, 0x60, 0x00, 0x00, // .
  0x20, 0x10, 0x08, 0x04, 0x02, // /
  0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
  0x00, 0x04, 0x02, 0x7F, 0x00, // 1
  0x42, 0x61, 0x51, 0x49, 0x46, // 2
  0x22, 0x41, 0x49, 0x49, 0x36, // 3
  0x18, 0x14, 0x12, 0x7F, 0x10, // 4
  0x27, 0x45, 0x45, 0x45, 0x39, // 5
  0x3E, 0x49, 0x49, 0x49, 0x32, // 6
  0x01, 0x01, 0x71, 0x09, 0x07, // 7
  0x36, 0x49, 0x49, 0x49, 0x36, // 8
  0x26, 0x49, 0x49, 0x49, 0x3E, // 9
  0x00, 0x36, 0x36, 0x00, 0x00, // :
  0x00, 0x56, 0x36, 0x00, 0x00, // ;
  0x08, 0x14, 0x22, 0x41, 0x00, // <
  0x14, 0x14, 0x14, 0x14, 0x14, // =
  0x00, 0x41, 0x22, 0x14, 0x08, // >
  0x02, 0x01, 0x51, 0x09, 0x06, // ?
  0x3E, 0x41, 0x59, 0x55, 0x5E, // @
  0x7E, 0x09, 0x09, 0x09, 0x7E, // A
  0x7F, 0x49, 0x49, 0x49, 0x36, // B
  0x3E, 0x41, 0x41, 0x41, 0x22, // C
  0x7F, 0x41, 0x41, 0x41, 0x3E, // D
  0x7F, 0x49, 0x49, 0x49, 0x41, // E
  0x7F, 0x09, 0x09, 0x09, 0x01, // F
  0x3E, 0x41, 0x41, 0x49, 0x3A, // G
  0x7F, 0x08, 0x08, 0x08, 0x7F, // H
  0x00, 0x41, 0x7F, 0x41, 0x00, // I
  0x30, 0x40, 0x40, 0x40, 0x3F, // J
  0x7F, 0x08, 0x14, 0x22, 0x41, // K
  0x7F, 0x40, 0x40, 0x40, 0x40, // L
  0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
  0x7F, 0x02, 0x04, 0x08, 0x7F, // N
  0x3E, 0x41, 0x41, 0x41, 0x3E, // O
  0x7F, 0x09, 0x09, 0x09, 0x06, // P
  0x1E, 0x21, 0x21, 0x21, 0x5E, // Q
  0x7F, 0x09, 0x09, 0x09, 0x76, // R
  0x26, 0x49, 0x49, 0x49, 0x32, // S
  0x01, 0x01, 0x7F, 0x01, 0x01, // T
  0x3F, 0x40, 0x40, 0x40, 0x3F, // U
  0x1F, 0x20, 0x40, 0x20, 0x1F, // V
  0x7F, 0x20, 0x10, 0x20, 0x7F, // W
  0x41, 0x22, 0x1C, 0x22, 0x41, // X
  0x03, 0x04, 0x78, 0x04, 0x03, // Y
  0x61, 0x51, 0x49, 0x45, 0x43, // Z
  0x00, 0x7F, 0x41, 0x00, 0x00, // [
  0x02, 0x04, 0x08, 0x10, 0x20, // backslash
  0x00, 0x00, 0x41, 0x7F, 0x00, // ]
  0x04, 0x02, 0x01, 0x02, 0x04, // ^
  0x40, 0x40, 0x40, 0x40, 0x40, // _
  0x00, 0x01, 0x02, 0x04, 0x00, // `
  0x20, 0x54, 0x54, 0x54, 0x78, // a
  0x7F, 0x44, 0x44, 0x44, 0x38, // b
  0x38, 0x44, 0x44, 0x44, 0x44, // c
  0x38, 0x44, 0x44, 0x44, 0x7F, // d
  0x38, 0x54, 0x54, 0x54, 0x18, // e
  0x04, 0x04, 0x7E, 0x05, 0x05, // f
  0x08, 0x54, 0x54, 0x54, 0x3C, // g
  0x7F, 0x08, 0x04, 0x04, 0x78, // h
  0x00, 0x44, 0x7D, 0x40, 0x00, // i
  0x20, 0x40, 0x44, 0x3D, 0x00, // j
  0x7F, 0x10, 0x28, 0x44, 0x00, // k
  0x00, 0x41, 0x7F, 0x40, 0x00, // l
  0x7C, 0x04, 0x78, 0x04, 0x78, // m
  0x7C, 0x08, 0x04, 0x04, 0x78, // n
  0x38, 0x44, 0x44, 0x44, 0x38, // o
  0x7C, 0x14, 0x14, 0x14, 0x08, // p
  0x08, 0x14, 0x14, 0x14, 0x7C, // q
  0x00, 0x7C, 0x08, 0x04, 0x04, // r
  0x48, 0x54, 0x54, 0x54, 0x20, // s
  0x04, 0x04, 0x3F, 0x44, 0x44, // t
  0x3C, 0x40, 0x40, 0x20, 0x7C, // u
  0x1C, 0x20, 0x40, 0x20, 0x1C, // v
  0x3C, 0x40, 0x30, 0x40, 0x3C, // w
  0x44, 0x28, 0x10, 0x28, 0x44, // x
  0x0C, 0x50, 0x50, 0x50, 0x3C, // y
  0x44, 0x64, 0x54, 0x4C, 0x44, // z
  0x00, 0x08, 0x36, 0x41, 0x41, // {
  0x00, 0x00, 0x7F, 0x00, 0x00, // |
  0x41, 0x41, 0x36, 0x08, 0x00, // }
  0x02, 0x01, 0x02, 0x04, 0x02, // ~
};

boolean blnConnected=false;
boolean blnBTInit=false;
boolean blnDayAdded=false;
boolean blnBTAddrFound=false;
boolean blnBTAddrDisp=false;
boolean blnNotificationUpdate=true;
boolean blnNestHome=true; // Home=true, Away=false
uint8_t intChargeStatus,intOldChargeStatus;
uint8_t currentHour,currentMin,currentSec;
uint8_t intGmail=0,intEmail=0;
uint16_t intRSS=0;
String currentToD;
Time currentTime;
Date currentDate;
uint32_t timer;
uint32_t NotificationStartTime=millis();
aci_evt_opcode_t status;
//int freeRAM=0;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("In init"));
  //Setup up vibration motor
  pinMode(VIBRATION,OUTPUT);
  
  //Setup charging monitor
  // Not connected:  BATTSTAT1 = HI, BATTSTAT2 = HI, intChargeStatus=0
  // Charging:  BATTSTAT1 = L, BATTSTAT2 = HI, intChargeStatus=1
  // Charge Complete:  BATTSTAT1 = HI, BATTSTAT2 = L,intChargeStatus=2
  pinMode(BATTSTAT1,INPUT);
  pinMode(BATTSTAT2,INPUT);
  intChargeStatus=0;
  intOldChargeStatus=0;
  updateChargeStatus();
  
  //Set default Nest to away
  blnNestHome=false;
  
  blnBTInit=false;
  Serial.println(F("Setting up SPI"));
  SPI.begin ();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setDataMode(SPI_MODE0);
  Serial.println(F("Setting up common pins"));
  pinMode(OLED_DATA,OUTPUT);
  pinMode(OLED_CLK,OUTPUT);
  Serial.println(F("Setting up Display Pins"));
  pinMode(OLED_DC,OUTPUT);
  pinMode(OLED_CS,OUTPUT);
  pinMode(OLED_RST,OUTPUT);
  Serial.println(F("Trigger reset of display"));
  digitalWrite(OLED_RST,HIGH);
  delay(1);
  digitalWrite(OLED_RST,LOW);
  delay(10);
  digitalWrite(OLED_RST,HIGH);
  Serial.println(F("Initializing Display"));
  // Init sequence for 128x64 OLED module
  ssd1306_command(SSD1306_DISPLAYOFF);          // 0xAE
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);  // 0xD5
  ssd1306_command(0x80);                        // the suggested ratio 0x80
  ssd1306_command(SSD1306_SETMULTIPLEX);        // 0xA8
  ssd1306_command(0x3F);
  ssd1306_command(SSD1306_SETDISPLAYOFFSET);    // 0xD3
  ssd1306_command(0x0);                         // no offset
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0);  // line #0
  ssd1306_command(SSD1306_CHARGEPUMP);          // 0x8D 
  ssd1306_command(0x14);
  ssd1306_command(SSD1306_MEMORYMODE);          // 0x20
  ssd1306_command(0x02);                        // 0x2 page mode
  ssd1306_command(SSD1306_SEGREMAP | 0x1);
  ssd1306_command(SSD1306_COMSCANDEC);
  ssd1306_command(SSD1306_SETCOMPINS);          // 0xDA
  ssd1306_command(0x12);
  ssd1306_command(SSD1306_SETCONTRAST);         // 0x81
  ssd1306_command(0xCF);
  ssd1306_command(SSD1306_SETPRECHARGE);        // 0xd9
  ssd1306_command(0xF1);
  ssd1306_command(SSD1306_SETVCOMDETECT);       // 0xDB
  ssd1306_command(0x40);
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME); // 0xA4
  ssd1306_command(SSD1306_NORMALDISPLAY);       // 0xA6
  Serial.println(F("Turning on display"));
  ssd1306_command(SSD1306_DISPLAYON);
  
  Serial.println(F("Clearing display"));
  blankDisplay(0);
  setCursor(0,0);
  
  blnBTInit=true;
  Serial.println(F("Initializing BTLE"));
  SPI.setBitOrder(LSBFIRST);
  digitalWrite(ADAFRUITBLE_RST,HIGH);
  delay(30);
  BTLEserial.begin();
  SPI.setBitOrder(MSBFIRST);
  
  blnConnected=false;
  blnDayAdded=false;
  blnBTAddrFound=false;
  blnBTAddrDisp=false;
  currentTime.setTime(12,0,0,0);
  currentHour=currentTime.getHours();
  currentMin=currentTime.getMinutes();
  currentSec=currentTime.getSeconds();
  currentToD="PM";
  currentDate.setDate(2014,1,1);
  
  timer=millis();
  
  writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,true);
  writeDate(currentDate.getDoW(),currentDate.getYear(),currentDate.getMonth(),currentDate.getDay());
  writeNotificationLine(0,0,0,1,intChargeStatus);

  Serial.println(F("Init complete"));
}

void loop()
{ 
  //Update Charge Status
  updateChargeStatus();
  if(intOldChargeStatus != intChargeStatus)
  {
    writeChargeStatus(intChargeStatus);
    intOldChargeStatus = intChargeStatus;
  }
  
  BTLEserial.pollACI();
  status=BTLEserial.getState();

  if (status == ACI_EVT_DEVICE_STARTED) 
  {
    if(status != laststatus)
    {
      Serial.println(F("* Advertising started"));
      writeBlankLine(3);
      setCursor(0,2);
      writeString("Ready...");
    }
    if(BTLEserial.strBTAddr.length()  > 0)
    {
      if(blnBTAddrDisp == false && blnBTAddrFound==true)
      {
        setCursor(50,2);
        Serial.println(F("Read the following from strBTAddr"));
        Serial.println(BTLEserial.strBTAddr);
        writeString(BTLEserial.strBTAddr);
        blnBTAddrDisp=true;
      }
      blnBTAddrFound=true;
    }
    blnConnected=false;
  }
  if (status == ACI_EVT_CONNECTED) 
  {
    if(status != laststatus)
    {
      Serial.println(F("* Connected!"));
      if (blnConnected == false)
      {
        writeBlankLine(2);
        writeBlankLine(3);
        blnBTAddrDisp=false;
        setCursor(0,3);
        //writeString("Connected.");
      }
    }
    blnConnected = true;
    String strOutput="";
    setCursor(0,3);
    // OK while we still have something to read, get a character and print it out
    while (BTLEserial.available() > 0) 
    {
      if((strOutput.length()!=0 && BTLEserial.peek() != '$') || (strOutput.length()==0 && BTLEserial.peek() == '$'))
      {
        strOutput.concat((char)BTLEserial.read());
      }
      else
      { 
        processBTInput(strOutput);
        strOutput="";
      }
    }
    if(strOutput.length() != 0)
    {
      processBTInput(strOutput);
    }
  }
  if (status == ACI_EVT_DISCONNECTED || status == ACI_EVT_INVALID || status == ACI_EVT_HW_ERROR || status == ACI_EVT_PIPE_ERROR) 
  {
      if(status != laststatus)
      {
        Serial.println(F("* Disconnected or advertising timed out"));
        writeBlankLine(3);
        setCursor(0,2);
        writeString("Ready...");
        blnConnected=false;
      }
      if(BTLEserial.strBTAddr.length()  > 0)
      {
        if(blnBTAddrDisp == false && blnBTAddrFound==true)
        {
          setCursor(50,2);
          Serial.println(F("Read the following from strBTAddr"));
          Serial.println(BTLEserial.strBTAddr);
          writeString(BTLEserial.strBTAddr);
          blnBTAddrDisp=true;
        }
        blnBTAddrFound=true;
      }
  }
  // OK set the last status change to this one
  laststatus = status;
    
  //Do Time computations
  if((millis()-timer) > 0)
  {
    uint32_t temp_timer;
    
    temp_timer=millis();
    currentTime.addMilliseconds(temp_timer-timer);
    timer=temp_timer;
  }
  else
  {
    uint32_t temp_timer;
    
    temp_timer=millis();
    currentTime.addMilliseconds((2^32)-timer+temp_timer);
    timer=temp_timer;
  }
  
  //Get correct AM/PM value
  if(currentTime.getHours() > 12)
  {
    currentToD="PM";
  }
  else
  {
    currentToD="AM";
  }
  
  //Print new Time
  if(currentHour==99)
  {
    writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,true);
    writeDate(currentDate.getDoW(),currentDate.getYear(),currentDate.getMonth(),currentDate.getDay());
  }
  else
  {
//    if(timer>millis())
//    {
//      //We just wrapped over
//      //Clear notification and update time in case we were in the middle of a notification.
//      NotificationStartTime=0;
//      writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,true);
//    }
//    else
//    {
//      if(NotificationStartTime+5000 > millis())
//      {
//        //Skip time update because notification is being displayed
//      }
//      else
//      {
        writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,false);
//      }
//
//    }
  }  
  
  // Update Date if required (time is midnight and we haven't already added a day)
  if(currentTime.getHours()==0 && currentTime.getMinutes()==0 && blnDayAdded==false)
  {
    currentDate.addDay();
    writeDate(currentDate.getDoW(),currentDate.getYear(),currentDate.getMonth(),currentDate.getDay());
    blnDayAdded=true;
  }
  
  // Reset day update flag at 12:01:00 AM each day
  if(currentTime.getHours()==0 && currentTime.getMinutes()==1 && blnDayAdded==true)
  {
    blnDayAdded=false;
  }

  if(blnNotificationUpdate == true)
  {
    writeNotificationLine(intGmail,intEmail,intRSS,blnNestHome,intChargeStatus);
  }
}

void updateChargeStatus()
{
  // Not connected:  BATTSTAT1 = HI, BATTSTAT2 = HI, intChargeStatus=0
  // Charging:  BATTSTAT1 = L, BATTSTAT2 = HI, intChargeStatus=1
  // Charge Complete:  BATTSTAT1 = HI, BATTSTAT2 = L,intChargeStatus=2
  
  if(digitalRead(BATTSTAT1) == HIGH && digitalRead(BATTSTAT2) == HIGH)
  {
    intChargeStatus=0;
  }
  else if (digitalRead(BATTSTAT1) == LOW && digitalRead(BATTSTAT2) == HIGH)
  {
    intChargeStatus=1;
  }
  else if (digitalRead(BATTSTAT1) == HIGH && digitalRead(BATTSTAT2) == LOW)
  {
    intChargeStatus=2;
  }
  else
  {
    intChargeStatus=0;
  }
}

void processBTInput(String s)
{
//  if(blnDebug == true)
//  {
//    writeBlankLine(2);
//    setCursor(0,2);
//    writeString(s);
//  }
  
  if(s.startsWith("$tm") == true)
  {
    processIncomingTime(s);
  }
  if(s.startsWith("$rs") == true)
  {
    void(* resetFunc) (void) = 0;//declare reset function at address 0
    resetFunc();
  }
  if(s.startsWith("$nf") == true)
  {
    processIncomingNotificationUpdate(s);
  }
  if(s.startsWith("$em") == true)
  {
    processIncomingEmail(s);
  }
  if(s.startsWith("$cl") == true)
  {
    processIncomingCall(s);
  }
//  if(s.startsWith("$g1") == true)
//  {
//    NotificationStartTime=millis();
//    processGeneralNotification(s,1);
//  }
//  if(s.startsWith("$g2") == true)
//  {
//    processGeneralNotification(s,2);
//  }
//  if(s.startsWith("$g3") == true)
//  {
//    processGeneralNotification(s,3);
//  }
//  if(s.startsWith("$db") == true)
//  {
//    blnDebug=!blnDebug;
//    writeBlankLine(2);
//  }
}

//void processGeneralNotification(String s,int line)
//{
//  writeBlankLine(line+2);
//  setCursor(0,line+2);
//  writeString(s.substring(3));
//  if(line == 3)
//  {
//    vibrate(1);
//    delay(3000);
//    writeBlankLine(3);
//    writeBlankLine(4);
//    writeBlankLine(5);
//    currentHour=99;
//  }
//}

void processIncomingTime(String s)
{
  blnDayAdded=false;
  
  currentTime.setTime((s.substring(11,13)).toInt(),(s.substring(13,15)).toInt(),(s.substring(15,17)).toInt(),0);
  currentDate.setDate((s.substring(3,7)).toInt(),(s.substring(7,9)).toInt(),(s.substring(9,11)).toInt());
  
//  writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,true);
//  writeDate(currentDate.getDoW(),currentDate.getYear(),currentDate.getMonth(),currentDate.getDay());
  currentHour=99;
}

void processIncomingNotificationUpdate(String s)
{
  uint8_t intStart,intEnd,intNest;
  
  intStart=3;
  intEnd=s.indexOf(',');
  intGmail=s.substring(intStart,intEnd).toInt();
  intStart=intEnd+1;
  intEnd=s.indexOf(',',intStart);
  intEmail=s.substring(intStart,intEnd).toInt();
  intStart=intEnd+1;
  intEnd=s.indexOf(',',intStart);
  intRSS=s.substring(intStart,intEnd).toInt();
  intStart=intEnd+1;
  intEnd=s.length();
  intNest=(s.substring(intStart,intEnd).toInt());
  
  if(intNest == 0)
  {
    blnNestHome=false;
  }
  else
  {
    blnNestHome=true;
  }
  
  blnNotificationUpdate=true;
}

void processIncomingEmail(String s)
{
  writeBlankLine(3);
  writeBlankLine(4);
  writeBlankLine(5);
  setCursor(0,3);
  writeString("Email from:");
  setCursor(0,4);
  writeBigString(s.substring(3));
  vibrate(1);
  delay(500);
  vibrate(1);
  delay(3000);
  writeBlankLine(3);
  writeBlankLine(4);
  writeBlankLine(5);
  currentHour=99;
}

void processIncomingCall(String s)
{
    writeBlankLine(3);
    writeBlankLine(4);
    writeBlankLine(5);
    setCursor(0,3);
    writeString("Call from:");
    setCursor(0,4);
    writeBigString(s.substring(3));
    vibrate(2);
    delay(3000);
    writeBlankLine(3);
    writeBlankLine(4);
    writeBlankLine(5);
    currentHour=99;
}

void writeNotificationLine(uint8_t numGmail,uint8_t numEmail, uint16_t numRSS,boolean nestHome,uint8_t chargeStatus)
{
  writeBlankLine(0);
  writeLogo(GMail_Logo,10,0,0);
  setCursor(13,0);
  writeString(String(numGmail));
  writeLogo(EMail_Logo,10,32,0);
  setCursor(45,0);
  writeString(String(numEmail));
  writeLogo(RSS_Logo,8,64,0);
  setCursor(74,0);
  writeString(String(numRSS));
  if(nestHome == true)
  {
    writeLogo(NestHome_Logo,10,95,0);
  }
  else
  {
    writeLogo(NestAway_Logo,10,95,0);
  }
  writeChargeStatus(chargeStatus);
  
  blnNotificationUpdate=false;
}

void writeChargeStatus(uint8_t chargeStatus)
{
  if(chargeStatus == 1)
  {
    writeLogo(Charging_Logo,10,117,0);
  }
  else if (chargeStatus == 2)
  {
    writeLogo(Charged_Logo,10,117,0);
  }
}

void ssd1306_command(uint8_t c)
{
  while(digitalRead(ADAFRUITBLE_RDY) == LOW && blnBTInit==true)
  {
    delay(10);
  }
  //Disable interrupts to make sure BT doesn't send data during display writes
  cli();

  digitalWrite(OLED_CS, HIGH);
  digitalWrite(OLED_DC, LOW);
  digitalWrite(OLED_CS, LOW);
  SPI.transfer(c);
  digitalWrite(OLED_CS, HIGH);
  
  //Reenable interrupts
  sei();
}

void ssd1306_data(uint8_t c) 
{
  while(digitalRead(ADAFRUITBLE_RDY) == LOW & blnBTInit==true)
  {
    delay(10);
   }
   
  //Disable interrupts to make sure BT doesn't send data during display writes
  cli();
  
  digitalWrite(OLED_CS, HIGH);
  digitalWrite(OLED_DC, HIGH);
  digitalWrite(OLED_CS, LOW);
  SPI.transfer(c);
  digitalWrite(OLED_CS, HIGH);
  
  //Reenable interrupts
  sei();
}

void blankDisplay(uint8_t color)
{
  uint16_t i,j;
  byte temp;
  
  if (color==0)
  {
    temp=(uint8_t)0;
  }
  else
  {
    temp=(uint8_t)255;
  }
  
  //Configure Display for Horizontal Addressing
  ssd1306_command(0x20);
  ssd1306_command(0b00100000);
  
  //Set cursor to 0,0
  ssd1306_command(0x21);
  ssd1306_command(0);
  ssd1306_command(127);
  ssd1306_command(SSD1306_SETLOWCOLUMN | (0 & 0XF));
  ssd1306_command(SSD1306_SETHIGHCOLUMN | (0 >> 4));
  ssd1306_command(SSD1306_SETSTARTPAGE | 0);
  
  //Send 1024 bytes to display
  for (i=0;i<1024;i++)
  {
    ssd1306_data(temp);
  }
  
  //Set display back to Page Addressing Mode
  ssd1306_command(0x20);
  ssd1306_command(0b10);
}

void writeDisplay(byte c, uint8_t page_x, uint8_t page_y)
{ 
  if ((page_x < 128) && (page_y < 8))
  {
    //Configure display location
    ssd1306_command(SSD1306_SETLOWCOLUMN | (page_x & 0XF));
    ssd1306_command(SSD1306_SETHIGHCOLUMN | (page_x >> 4));
    ssd1306_command(SSD1306_SETSTARTPAGE | page_y);
    
    //Send byte to display
    ssd1306_data(c);
  }
}

void writeDate(uint8_t dow,uint16_t year,uint8_t month,uint8_t day)
{
  String temp="";
  
  switch(dow)
  {
    case 0:
      temp.concat("SUN ");
      temp=temp+year+" / "+month+" / "+day;
      break;
    case 1:
      temp.concat("MON ");
      temp=temp+year+" / "+month+" / "+day;
      break;
    case 2:
      temp.concat("TUE ");
      temp=temp+year+" / "+month+" / "+day;
      break;
    case 3:
      temp.concat("WED ");
      temp=temp+year+" / "+month+" / "+day;
      break;
    case 4:
      temp.concat("THU ");
      temp=temp+year+" / "+month+" / "+day;
      break;
    case 5:
      temp.concat("FRI ");
      temp=temp+year+" / "+month+" / "+day;
      break;
    case 6:
      temp.concat("SAT ");
      temp=temp+year+" / "+month+" / "+day;
      break;
  }
  
  writeBlankLine(7);
  writeString(temp,9,7);
}

void writeTime(uint8_t hour,uint8_t minute,uint8_t second,String ToD,boolean fullRefresh)
{
  String strHour,strMin,strSec;
  
  if(fullRefresh == true)
  {
    currentHour=99;
    currentMin=99;
    currentSec=99;
    writeBlankLine(4);
    writeBlankLine(5);
  }
  
  if(hour != currentHour)
  {
    currentHour=hour;
    strHour=String(hour);
    Serial.print(F("strHour="));
    Serial.println(strHour);
    if(strHour.length() < 2)
    {
      strHour=' '+strHour;
    }
    setCursor(5,4);
    writeBigString(strHour);
  }
  setCursor(30,4);
  writeBigChar(':');
  if(minute != currentMin)
  {
    currentMin=minute;
    strMin=String(minute);
    Serial.print(F("strMin="));
    Serial.println(strMin);
    if(strMin.length() <2)
    {
      strMin='0'+strMin;
    }
    setCursor(41,4);
    writeBigString(strMin);
  }
  setCursor(63,4);
  writeBigChar(':');
  if(second != currentSec)
  {
    currentSec=second;
    strSec=String(second);
    if(strSec.length() <2)
    {
      strSec='0'+strSec;
    }
    setCursor(74,4);
    writeBigString(strSec);
  }
  setCursor(107,4);
  writeString(ToD);
}

void writeString(String s)
{
  uint8_t i=0;
  
  while (i<s.length())
  {
    writeChar(s.charAt(i));
    i++;
  } 
}

void writeBigString(String s)
{
  uint8_t i=0;
  
  while (i<s.length())
  {
    writeBigChar(s.charAt(i));
    i++;
  } 
}

void writeBlankLine(uint8_t y)
{
  uint8_t i;
  byte temp=0x00;
  
  for(i=0;i<128;i++)
  {
    writeDisplay(temp,i,y);
  }
}

void writeString(String s,uint8_t x,uint8_t y)
{
  uint8_t i=0;
  
  setCursor(x,y);
  while (i<s.length())
  {
    writeChar(s.charAt(i));
    i++;
  } 
}

void writeLogo(unsigned char logo[],uint8_t width,uint8_t x,uint8_t y)
{
  uint8_t i=0;
  
  setCursor(x,y);
  while(i<width)
  {
    writeDisplay(pgm_read_byte(&(logo[i])),x+i,y);
    i++;
  }
}

void setCursor(uint16_t x,uint16_t y)
{
  if ((x >= 0) & (x<128) & (y>=0) & (y<8))
  {
    xpos=(uint8_t)x;
    ypos=(uint8_t)y;
  }
  else
  {
    if (xpos < 0)
    {
      xpos=0;
    }
    if (xpos >= 128)
    {
      xpos=0;
      ypos=(uint8_t)ypos+1; 
    }
    if(ypos >= 8)
    {
      ypos=0; 
    }
  }
}

void writeBigChar(char c)
{
  uint16_t i,j,k,index;
  byte top,bottom,target_char;

  index=5*(c-32);
  
  for(i=index;i<index+5;i++)
  {
    if ((128-xpos) < 10)
    {
      setCursor(0,ypos+2);
    }
    target_char=pgm_read_byte(&(font[i]));
    for(k=0;k<4;k++)
    {
      bitWrite(top,(2*k),bitRead(target_char,k));
      bitWrite(top,(2*k)+1,bitRead(target_char,k));
      bitWrite(bottom,(2*k),bitRead(target_char,k+4));
      bitWrite(bottom,(2*k)+1,bitRead(target_char,k+4));
    }
    for(j=0;j<2;j++)
    {
      writeDisplay(top,xpos,ypos);
      writeDisplay(top,xpos+1,ypos);
      writeDisplay(bottom,xpos,ypos+1);
      writeDisplay(bottom,xpos+1,ypos+1);
    }
    setCursor(xpos+2,ypos);
  } 
  setCursor(xpos+1,ypos);
}

void writeChar(char c)
{
  uint16_t i,index;

  index=5*(c-32);
  
  for(i=index;i<index+5;i++)
  {
    if ((128-xpos) < 5)
    {
      setCursor(0,ypos+1);
    }
    writeDisplay(pgm_read_byte(&(font[i])),xpos,ypos);
    setCursor(xpos+1,ypos);
  }
  setCursor(xpos+1,ypos);
}

void debugVariable(String s,long unsigned int val,char type)
{
  Serial.print(s);
  if(type == 'b')
  {
    Serial.print(val,BIN);
  }
  if(type == 'd')
  {
    Serial.print(val,DEC);
  }
  if(type == 'h')
  {
    Serial.print(val,HEX);
  }
  Serial.println("");
}

void debugVariable(String s,String val)
{
  Serial.print(s);
  Serial.print(val);
  Serial.println("");
}

int free_ram()
{
  extern int __heap_start, *__brkval;
  int v;
  
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void vibrate(uint8_t duration_sec)
{
  digitalWrite(VIBRATION,HIGH);
  delay(duration_sec*1000);
  digitalWrite(VIBRATION,LOW);
}
