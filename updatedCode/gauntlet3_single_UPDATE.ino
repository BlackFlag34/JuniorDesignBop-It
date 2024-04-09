// Comms: Gauntlet #3


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
// #include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <IRremote.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <Adafruit_PCF8574.h>
// #include "mp3tf16p.h"

// TFT Display Definitions
// SDA = 11
// SCL = 13
#define TFT_DC 9                                            
#define TFT_CS 10
#define TFT_BL 12
#define TFT_RST 7
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// define accelerometer
MPU6050 mpu6050(Wire);

// define mp3 player
// MP3Player mp3(A7,A6);   
// #define ArrowDraw 1
// #define FireBall 2
// #define SwordSmite 3
// #define ArrowLaunch 4

// // OLED Display Definitions
// #define OLED_MOSI  11         //D1
// #define OLED_CLK   12         //D0
// #define OLED_DC    9
// #define OLED_CS    8
// #define OLED_RESET 10
// Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// IR communication setup and variables
#define IRpin 5
uint8_t IR_hexValue, IR_repeats = 3;

// IC_Extender Varaible
Adafruit_PCF8574 pcf;

// LoRa as SoftwareSerial
SoftwareSerial lora(3,2);                                   

// Time Variable
unsigned long curTime, timerTime, delTime = 10000;

// character data (characterSelect = 1, 2, 3 => Knight, Mage, Archer) (action = 1, 2, 3 => Kill Monster, Move Monster, Special Attack)
int characterSelector = A0, characterData = 0, characterSelect = 0, action = 0;

// indicators: startup, active, special_1, special_2, special_3, haptic   ;   startup also indicates sensor reading
// int startupLight = A3, haptic = A2;

int Magic3 = A7, Magic2 = A6, Magic1 = A3, Magic0 = A2;

// buttons and sensor requirement variable
int characterSendButton = 6, sensorButton = A1;

// truth for holds
bool sensor = false, gameStart = true, fireballStatus = false, arrowStatus = false, swordStatus = false;

// round identifier
double monsterRound = 0;

// debugging tools
int transmissionNumber = 0;

// arrow
int arrowIn = 4;

// 
int haptic = 0;
int startupLight = 1;
int mD = 2;
// int mp31 = 3;
// int mp32 = 4;

void setup() {

  Serial.begin(115200);
  lora.begin(115200);
  IrSender.begin(IRpin);

  // accelerometer initialize
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);  

  // Initialize I2C Extender
  pcf.begin(0x20, &Wire);

  pcf.pinMode(haptic, OUTPUT); // Haptic
  pcf.pinMode(startupLight, OUTPUT);
  pcf.pinMode(mD, OUTPUT);
  // pcf.pinMode(mp31, OUTPUT);
  // pcf.pinMode(mp32, OUTPUT);

  // mp3.initialize();

  tft.begin();
  // display.begin(SSD1306_SWITCHCAPVCC);
  pinMode(TFT_BL, OUTPUT);  digitalWrite(TFT_BL, HIGH);   // Backlight on
  displayGameStartup();

  pinMode(3, INPUT);  pinMode(2, OUTPUT);  // LoRa pin definitions

  pinMode(characterSelector,INPUT); pinMode(characterSendButton, INPUT);  pinMode(sensorButton, INPUT);

 // pinMode(startupLight, OUTPUT); pinMode(haptic, OUTPUT);
  pcf.digitalWrite(startupLight, LOW);

  pinMode(arrowIn, INPUT);

  displayGameStartup();

  while (characterSelect==0){ getCharacter(); }           // get character type
  if (characterSelect == 1){ Serial.println(F("Character Selected: Knight"));  }
  else if (characterSelect == 2){ Serial.println(F("Character Selected: Mage"));  }
  else if (characterSelect == 3){ Serial.println(F("Character Selected: Archer"));  }
  else {  (Serial.println(F("ERROR")));  }

  bool sent = false;
  while(sent == false){ if(digitalRead(characterSendButton)){ sendIR();  sent = true; } }
  Serial.println(F("IR Sent, Value:"));
  Serial.println(IR_hexValue);
  
  pcf.digitalWrite(startupLight, HIGH);
}

void loop() 
{
  if (gameStart == true){ receiveLoRa(); }
  else if (action == 0){ receiveLoRa(); }
  else { playRound(); }
}

void getCharacter()
{
  characterData = map(analogRead(characterSelector),0,1024,0,255);
  if (characterData < 75){  characterSelect = 1;  }
  else if ((characterData > 75)&&(characterData < 150)){ characterSelect = 2; }
  else { characterSelect = 3; }
}

void playRound()
{ 
  Serial.println(F("Playing Round Now"));
  // String sending = "Round "+String(monsterRound)+" Start";
  if (action == 1){ displayInfo(F("Round Start"), F("Kill Monster"), F("")); sensor = true;  }
  else if (action == 2){  displayInfo(F("Round Start"), F("Move Monster"), F("")); sensor = true;  }
  else if (action == 3){
    displayInfo(F("Round Start"), F("SPECIAL POWER"), F(""));
    Serial.println(F("Gathering Sensor Data"));
    // sendSensorRequest();
    sensor = false;   readSensor();    
  }
}

// void sendSensorRequest()
// {
//   byte attack = 0;
//   attack = 25;
//   Serial.write(attack);

//   Serial.println(F("Sending Sensor Request:"));
//   Serial.println(attack);
// }

void readSensor()
{
  Serial.println(F("Receiving Sensor Information:"));
  pcf.digitalWrite(startupLight, LOW);
  if (characterSelect == 1)
  {
    Serial.println(F("Checking Sensor..."));
    while (swordStatus == false)
    {
      smiteSword();
    }
    Serial.println(F("sensor data received"));
    while (sensor == false){ if(digitalRead(sensorButton)){ sendIR(); 
    Serial.println(F("IR Sent, Value:"));
    Serial.println(IR_hexValue);
    delay(50); sensor = true; swordStatus = false; action = 0; } }
  }
  if (characterSelect == 2)
  {
      Serial.println(F("Checking Sensor..."));
    //while (fireballStatus == false)
    {
      Serial.println(F("firing!"));
      magicCast();
    }
    Serial.println(F("sensor data received"));
    while (sensor == false){ if(digitalRead(sensorButton)){ sendIR(); delay(50); sensor = true; fireballStatus = false; action = 0; } }
    }

  if (characterSelect == 3)
  {
    Serial.println(F("Checking Sensor..."));
    while (arrowStatus == false)
    {
      Serial.println(F("arrowing!"));
      drawArrow();
    }
    Serial.println(F("sensor data received"));
    while (sensor == false){ if(digitalRead(sensorButton)){ sendIR(); delay(50); sensor = true; arrowStatus = false; action = 0; } }
  }
  pcf.digitalWrite(startupLight, HIGH);
}

void sendIR()
{
  if (gameStart == true)
  {
    if (characterSelect == 1){ IR_hexValue = 0x12; }
    else if (characterSelect == 2){ IR_hexValue = 0x13; }
    else if (characterSelect == 3){ IR_hexValue = 0x14; } 
  }
  else
  {
    if ((characterSelect == 1)&&(swordStatus == true)){ IR_hexValue = 0x15; }
    else if ((characterSelect == 1)&&(swordStatus == false)){ IR_hexValue = 0x16; }
    else if ((characterSelect == 2)&&(fireballStatus == true)){ IR_hexValue = 0x17; }
    else if ((characterSelect == 2)&&(fireballStatus == false)){ IR_hexValue = 0x18; }
    else if ((characterSelect == 3)&&(arrowStatus == true)){ IR_hexValue = 0x19; }
    else if ((characterSelect == 3)&&(arrowStatus == false)){ IR_hexValue = 0x1A; }
  }  
  IrSender.sendNEC(0x00, IR_hexValue, IR_repeats);
}

void receiveLoRa()
{
  String incomingTransmission;
  if ( lora.available() > 0 )
  {
    incomingTransmission = lora.readString();
    Serial.println(F("LoRa Transmission Received:"));
    Serial.println(incomingTransmission);

    // String address = getValue(incomingTransmission, ',', 0);                      // address
    // String length = getValue(incomingTransmission, ',', 1);                       // data length
    String information = getValue(incomingTransmission, ',', 2);                     // transmission information
    // String rssi = getValue(incomingTransmission, ',', 3);                         //RSSI
    // String snr = getValue(incomingTransmission, ',', 4);                          //SNR

    Serial.println(F("LoRa Information Parsed:"));
    Serial.println(information);

    // String inID = getValue(information, '%', 0);                                  // identify sender
    String transmissionType = getValue(information, '%', 1);               
    String roundNumber = getValue(information, '%', 2);
    String actionCode_1 = getValue(information, '%', 3);
    String actionCode_2 = getValue(information, '%', 4);
    String actionCode_3 = getValue(information, '%', 5);

    Serial.println(F("LoRa Transmission Type:"));
    Serial.println(transmissionType);
    Serial.println(F("LoRa Round Number:"));
    Serial.println(roundNumber);
    Serial.println(F("LoRa Action Code:"));
    Serial.println(actionCode_3);

    if ((transmissionType == "CC")&&(characterSelect == 1)){ displayInfo(F("Confirmed"), "Knight", "Game Starting");  gameStart = false; }
    if ((transmissionType == "CC")&&(characterSelect == 2)){ displayInfo(F("Confirmed"), "Choice: Mage", "Game Starting");  gameStart = false; }
    if ((transmissionType == "CC")&&(characterSelect == 3)){ displayInfo(F("Confirmed"), "Choice: Archer", "Game Starting");  gameStart = false; }
    if ((actionCode_1 != actionCode_2)&&(actionCode_1 != actionCode_3)&&(actionCode_2 != actionCode_3))
    {
      if(   ((actionCode_1 == "M")||(actionCode_1 == "K")||(actionCode_1 == "S"))
          &&((actionCode_2 == "M")||(actionCode_2 == "K")||(actionCode_2 == "S"))
          &&((actionCode_3 == "M")||(actionCode_3 == "K")||(actionCode_3 == "S"))   )
      {
        if ((transmissionType == "R")&&(actionCode_3 == "K")){
          action = 1; 
          monsterRound = roundNumber.toDouble();
          for (int i=0 ; i <= 1 ; i++)
          {
            pcf.digitalWrite(haptic, LOW);
            delay(100);
            pcf.digitalWrite(haptic, HIGH);
            delay(100);
          }
          pcf.digitalWrite(haptic, LOW);
          delay(200);
          pcf.digitalWrite(haptic, HIGH);
          delay(100);      
        }        
        else if ((transmissionType == "R")&&(actionCode_3 == "M")){ 
          action = 2; 
          monsterRound = roundNumber.toDouble();
          for (int i=0 ; i <= 1 ; i++)
          {
            pcf.digitalWrite(haptic, LOW);
            delay(100);
            pcf.digitalWrite(haptic, HIGH);
            delay(100);
          }
          pcf.digitalWrite(haptic, LOW);
          delay(200);
          pcf.digitalWrite(haptic, HIGH);
          delay(100);           
        }
        else if ((transmissionType == "R")&&(actionCode_3 == "S")){
          action = 3; 
          monsterRound = roundNumber.toDouble();
          for (int i=0 ; i <= 1 ; i++)
          {
            pcf.digitalWrite(haptic, LOW);
            delay(100);
            pcf.digitalWrite(haptic, HIGH);
            delay(100);
          }
          pcf.digitalWrite(haptic, LOW);
          delay(200);
          pcf.digitalWrite(haptic, HIGH);
          delay(100);            
        }
      }
    }

    Serial.println(F("LoRa Results:"));
    if (action == 1){ Serial.println(F("Action: Kill Monster")); }
    else if (action == 2){ Serial.println(F("Action: Move Monster")); }
    else if (action == 3){ Serial.println(F("Action: Special Attack")); }
    else { Serial.println(F("ERROR")); }
    Serial.println(F("Round: "));
    Serial.println(monsterRound);
    Serial.println(F("LoRa Action Code:"));
    Serial.println(actionCode_3);

  }
}

void displayInfo(String line1, String line2, String line3){
  tft.setRotation(3);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(70, 30);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(2);
  tft.println(line1);
  tft.setCursor(42, 75);
  tft.setTextColor(GC9A01A_RED); tft.setTextSize(2);
  tft.println(line2);
  tft.setCursor(30, 95);
  tft.setTextColor(GC9A01A_RED); tft.setTextSize(4);
  tft.println("");
  tft.setCursor(53, 145);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(line3);
  tft.setCursor(35, 165);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println("");
  tft.println();
}

void displayGameBegin(String characterType){
  tft.setRotation(3);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(70, 30);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(2);
  tft.println(F("Confirmed!"));
  tft.setCursor(42, 75);
  tft.setTextColor(GC9A01A_RED); tft.setTextSize(2);
  tft.println(F("Character:"));
  tft.setCursor(30, 95);
  tft.setTextColor(GC9A01A_RED); tft.setTextSize(4);
  tft.println(characterType);
  tft.setCursor(53, 145);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Get Ready to"));
  tft.setCursor(35, 165);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Play!"));
  tft.println();
}

void displayGameStartup(){
  tft.setRotation(3);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(70, 30);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(2);
  tft.println(F("Player 3"));
  tft.setCursor(42, 75);
  tft.setTextColor(GC9A01A_RED); tft.setTextSize(2);
  tft.println(F("You're Playing"));
  tft.setCursor(30, 95);
  tft.setTextColor(GC9A01A_RED); tft.setTextSize(4);
  tft.println(F("SKRIMSLI"));
  tft.setCursor(53, 145);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Choose Your"));
  tft.setCursor(35, 165);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Character Type"));
  tft.println();
}

int irC = 0;
void magicCast(void)
{
  curTime = millis();
  Serial.println(curTime);
  Serial.println("MAGIC!");
  while(irC < 4)
  {
    if (curTime + delTime == millis())
    {
      // Time's Up, Fireball Status Should be False and break out of the while loop
      fireballStatus = false;
      irC += 5;
    }
    pcf.digitalWrite(mD, LOW);
    if (analogRead(Magic0) < 600 && irC == 0)
    {
      // Add Light to Indicate First IR Read NeoP
      Serial.println("A7 Read");
      irC++;
    }

    if (analogRead(Magic1) < 600 && irC == 1)
    {
      // Add Light to Indicate Second IR Read NeoP
      Serial.println("A6 Read");
      irC++;
    }

    if (analogRead(Magic2) < 600 && irC == 2)
    {
      // Add Light to Indicate Third IR Read NeoP
      Serial.println("A3 Read");
      irC++;
    }

    if (analogRead(Magic3) < 600 && irC == 3)
    {
      // Add Light to Indicate Fourth IR Read NeoP
      Serial.println("A2 Read");
      irC++;
      pcf.digitalWrite(startupLight, LOW);
      fireballStatus = true;
      pcf.digitalWrite(mD, HIGH);
    }
  }
  irC = 0;
  pcf.digitalWrite(mD, HIGH);
}

void drawArrow(void)
{
  if (digitalRead(arrowIn) == HIGH)
  {
    Serial.println(F("ARROW!"));
    // mp3.playTrackNumber(1, 25);
    while(digitalRead(arrowIn) == HIGH);
    // mp3.playTrackNumber(4, 25);
    pcf.digitalWrite(startupLight, LOW);
    arrowStatus = true;
  }
}

void smiteSword(void)
{
  Serial.println("SWORD!");
  mpu6050.update();
  Serial.println(mpu6050.getAccZ());
  Serial.println("");
  if (mpu6050.getAccZ() > 1.90)
  {
    pcf.digitalWrite(startupLight, LOW);
    // mp3.playTrackNumber(SwordSmite, 25, false);
    swordStatus = true;
  }
}

// parse data (this is the one function I did not write)
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) 
    {
      if (data.charAt(i) == separator || i == maxIndex) 
      {
        found++;
        strIndex[0] = strIndex[1] + 1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
      }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}