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

#include <SSD1306ASCII.h>
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

uint8_t xpos=0,ypos=0;

static unsigned char __attribute__ ((progmem)) GMail_Logo[]={0xFF,0xFF,0x06,0x06,0x0C,0x0C,0x06,0x06,0xFF,0xFF};
static unsigned char __attribute__ ((progmem)) EMail_Logo[]={0xFF,0x83,0x85,0x89,0x91,0x91,0x89,0x85,0x83,0xFF};
static unsigned char __attribute__ ((progmem)) RSS_Logo[]={0xDB,0xDB,0x1B,0xF3,0xEE,0x0E,0xFC,0xF0};
  
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

boolean blnConnected;
boolean blnBTInit;
boolean blnDayAdded;
uint8_t currentHour,currentMin,currentSec;
String currentToD;
Time currentTime;
Date currentDate;
uint32_t timer;
aci_evt_opcode_t status;
int freeRAM=0;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("In init"));
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
  currentHour=99;
  currentMin=99;
  currentSec=99;
  currentTime.setTime(12,0,0,0);
  currentToD="PM";
  currentDate.setDate(2014,1,1);
  
  timer=millis();
  
  writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,false);
  writeDate(currentDate.getDoW(),currentDate.getYear(),currentDate.getMonth(),currentDate.getDay());
  writeNotificationLine(0,0,0);

  Serial.println(F("Init complete"));
}

void loop()
{ 
  BTLEserial.pollACI();
  status=BTLEserial.getState();

  if (status == ACI_EVT_DEVICE_STARTED) 
  {
    if(status != laststatus)
    {
      Serial.println(F("* Advertising started"));
      writeBlankLine(3);
      setCursor(0,2);
      writeString("Listening...");
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
  if (status == ACI_EVT_DISCONNECTED) 
  {
      if(status != laststatus)
      {
        Serial.println(F("* Disconnected or advertising timed out"));
        writeBlankLine(3);
        setCursor(0,2);
        writeString("Listening...");
        blnConnected=false;
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
  writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,false);
  
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
}

void processBTInput(String s)
{
  writeBlankLine(2);
  setCursor(0,2);
  writeString(s);
  
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
}

void processIncomingTime(String s)
{
  uint8_t month,day,hour,minute,second;
  uint16_t year;
  
  blnDayAdded=false;
  
  year=(s.substring(3,7)).toInt();
  month=(s.substring(7,9)).toInt();
  day=(s.substring(9,11)).toInt();
  hour=(s.substring(11,13)).toInt();
  minute=(s.substring(13,15)).toInt();
  second=(s.substring(15,17)).toInt();
  
  currentTime.setTime(hour,minute,second,0);
  currentDate.setDate(year,month,day);
  
  writeTime(currentTime.getAMPMHours(),currentTime.getMinutes(),currentTime.getSeconds(),currentToD,true);
  writeDate(currentDate.getDoW(),currentDate.getYear(),currentDate.getMonth(),currentDate.getDay());
}

void processIncomingNotificationUpdate(String s)
{
  uint8_t intGmail=0,intEmail=0, intRSS=0,intStart,intEnd;
  
  intStart=3;
  intEnd=s.indexOf(',');
  intGmail=(s.substring(intStart,intEnd).toInt());
  intStart=intEnd+1;
  intEnd=s.indexOf(',',intStart);
  intEmail=(s.substring(intStart,intEnd).toInt());
  intStart=intEnd+1;
  intEnd=s.length();
  intRSS=(s.substring(intStart,intEnd).toInt());
  
  writeNotificationLine(intGmail,intEmail,intRSS);
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
  delay(3000);
  writeBlankLine(3);
  writeBlankLine(4);
  writeBlankLine(5);
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
    delay(3000);
    writeBlankLine(3);
    writeBlankLine(4);
    writeBlankLine(5);
}

void writeNotificationLine(uint8_t numGmail,uint8_t numEmail, uint8_t numRSS)
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
  String temp,strDoW;
  
  switch(dow)
  {
    case 0:
      strDoW="SUN";
      break;
    case 1:
      strDoW="MON";
      break;
    case 2:
      strDoW="TUE";
      break;
    case 3:
      strDoW="WED";
      break;
    case 4:
      strDoW="THU";
      break;
    case 5:
      strDoW="FRI";
      break;
    case 6:
      strDoW="SAT";
      break;
  }
  
  temp=strDoW+" "+year+" / "+month+" / "+day;
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
