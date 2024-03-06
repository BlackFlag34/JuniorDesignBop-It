// Comms Gauntlet #2

#include <SPI.h>
#include <Adafruit_GFX.h>
// #include <Adafruit_GC9A01A.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <IRremote.h>

// // TFT Display Definitions
// #define TFT_DC 9                                            
// #define TFT_CS 10
// #define TFT_BL 12
// #define TFT_RST 7
// Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

#define OLED_MOSI  11         //D1
#define OLED_CLK   12         //D0
#define OLED_DC    9
#define OLED_CS    8
#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// IR communication setup and variables
#define IRpin       4
uint8_t IR_hexValue, IR_repeats = 3;

// LoRa as SoftwareSerial
SoftwareSerial lora(3,2);                                   

// character data (characterSelect = 1, 2, 3 => Knight, Mage, Archer) (action = 1, 2, 3 => Kill Monster, Move Monster, Special Attack)
int characterSelector = A0, characterData = 0, characterSelect = 0, action = 0;

// indicators: startup, active, special_1, special_2, special_3, haptic   ;   startup also indicates sensor reading
int startupLight = A5, activeGameLight = A4, specialLight_1 = A3, specialLight_2 = A2, specialLight_3 = A1;

// // haptic feedback
// haptic = A2;

// buttons and sensor requirement variable
int characterSendButton = 13, sensorButton = 5;

// truth for holds
bool sensor = false, gameStart = true, smiteStatus = false, fireballStatus = false, arrowStatus = false;

// round identifier
double monsterRound = 0;

// // debugging tools
// int transmissionNumber = 0;


void setup() {

  Serial.begin(115200);
  lora.begin(115200);
  IrSender.begin(IRpin);

  // tft.begin();
  display.begin(SSD1306_SWITCHCAPVCC);
  // pinMode(TFT_BL, OUTPUT);  digitalWrite(TFT_BL, HIGH);   // Backlight on

  pinMode(3, INPUT);  pinMode(2, OUTPUT);  // LoRa pin definitions

  pinMode(characterSelector,INPUT); pinMode(characterSendButton, INPUT);  pinMode(sensorButton, INPUT);

  pinMode(startupLight, OUTPUT);  pinMode(activeGameLight, OUTPUT);
  // pinMode(haptic, OUTPUT);
  digitalWrite(startupLight, HIGH); digitalWrite(activeGameLight, LOW);

  pinMode(specialLight_1, OUTPUT);  pinMode(specialLight_2, OUTPUT);  pinMode(specialLight_3, OUTPUT);
  digitalWrite(specialLight_1, LOW);  digitalWrite(specialLight_2, LOW);  digitalWrite(specialLight_3, LOW);

  displayGameStartup();

  while (characterSelect==0){ getCharacter(); }

  bool sent = false;
  while(sent == false){ if(digitalRead(characterSendButton)){ sendIR();  sent = true; } }

  digitalWrite(startupLight, LOW);  digitalWrite(activeGameLight, HIGH);
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
  if (action == 1){ displayInfo("Round "+String(monsterRound)+" Start", F("Your Action: "), F("Kill Monster")); }
  else if (action == 2){  displayInfo("Round "+String(monsterRound)+" Start", F("Your Action: "), F("Move Monster")); }
  else if (action == 3){
    digitalWrite(specialLight_1, HIGH); delay(300); digitalWrite(specialLight_1, LOW);
    digitalWrite(specialLight_2, HIGH); delay(300); digitalWrite(specialLight_2, LOW);
    digitalWrite(specialLight_3, HIGH); delay(300); digitalWrite(specialLight_3, LOW);
    digitalWrite(specialLight_1, HIGH); delay(300); digitalWrite(specialLight_1, LOW);
    digitalWrite(specialLight_2, HIGH); delay(300); digitalWrite(specialLight_2, LOW);
    digitalWrite(specialLight_3, HIGH); delay(300); digitalWrite(specialLight_3, LOW);

    displayInfo("Round "+String(monsterRound)+" Start", F("Your Action: "), F("SPECIAL POWER")); }

  // sendSensorRequest();
  sensor = false;   readSensor();
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
  digitalWrite(startupLight, HIGH);
  if (characterSelect == 1)
  {
    while (sensor == false)
    {
      if (digitalRead(sensorButton))
      {
        smiteStatus = true;
        sendIR();
        delay(50);
        smiteStatus = false;
        sensor = true;
        action = 0;
      }
    }
  }
  if (characterSelect == 2)
  {
    while (sensor == false)
    {
      if (digitalRead(sensorButton))
      {
        fireballStatus = true;
        sendIR();
        delay(50);
        fireballStatus = false;
        sensor = true;
        action = 0;
      }
    }
  }
  if (characterSelect == 3)
  {
    bool rec = false;   
    while (rec == false)
    {
      if(Serial.available() > 0)
      {
        byte data = 0;
        while (data != 52){   int x = Serial.read();    data = x;  }
        if (data == 52){ digitalWrite(startupLight, HIGH);  delay(1000);  digitalWrite(startupLight, LOW);  rec = true; }
      }
      arrowStatus = true;    
      while (sensor == false){ if(digitalRead(sensorButton)){ sendIR(); delay(50); sensor = true; arrowStatus = false; action = 0; } }
    }
  }
  digitalWrite(startupLight, LOW);
}

void sendIR()
{
  if (gameStart == true)
  {
    if (characterSelect == 1){ IR_hexValue = 0x09; }
    else if (characterSelect == 2){ IR_hexValue = 0x0A; }
    else if (characterSelect == 3){ IR_hexValue = 0x0B; } 
  }
  else
  {
    if ((characterSelect == 1)&&(smiteStatus == true)){ IR_hexValue = 0x0C; }
    else if ((characterSelect == 1)&&(smiteStatus == false)){ IR_hexValue = 0x0D; }
    else if ((characterSelect == 2)&&(fireballStatus == true)){ IR_hexValue = 0x0E; }
    else if ((characterSelect == 2)&&(fireballStatus == false)){ IR_hexValue = 0x0F; }
    else if ((characterSelect == 3)&&(arrowStatus == true)){ IR_hexValue = 0x10; }
    else if ((characterSelect == 3)&&(arrowStatus == false)){ IR_hexValue = 0x11; }
  }  
  IrSender.sendNEC(0x00, IR_hexValue, IR_repeats);
}

void receiveLoRa()
{
  String incomingTransmission;
  if ( lora.available() > 0 )
  {
    int lastRound = monsterRound;
    // transmissionNumber++;
    incomingTransmission = lora.readString();

    // String address = getValue(incomingTransmission, ',', 0);                      // address
    // String length = getValue(incomingTransmission, ',', 1);                       // data length
    String information = getValue(incomingTransmission, ',', 2);                     // transmission information
    // String rssi = getValue(incomingTransmission, ',', 3);                         //RSSI
    // String snr = getValue(incomingTransmission, ',', 4);                          //SNR

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

      if ((transmissionType == "CC")&&(characterSelect == 1)){ displayInfo(F("Character Confirmed."), F("Character Type: "), F("Knight"));  gameStart = false; }
      else if ((transmissionType == "CC")&&(characterSelect == 2)){ displayInfo(F("Character Confirmed."), F("Character Type: "), F("Mage"));  gameStart = false; }
      else if ((transmissionType == "CC")&&(characterSelect == 3)){ displayInfo(F("Character Confirmed."), F("Character Type: "), F("Archer"));  gameStart = false; }

      if ((actionCode_1 != actionCode_2)&&(actionCode_1 != actionCode_3)&&(actionCode_2 != actionCode_3))
      {
        if(   ((actionCode_1 == "M")||(actionCode_1 == "K")||(actionCode_1 == "S"))
            &&((actionCode_2 == "M")||(actionCode_2 == "K")||(actionCode_2 == "S"))
            &&((actionCode_3 == "M")||(actionCode_3 == "K")||(actionCode_3 == "S"))   )
        {
        if ((transmissionType == "R")&&(actionCode_2 == "K")&&(roundNumber.toDouble() == (lastRound+1)))
        { 
          action = 1; 
          monsterRound = roundNumber.toDouble();
          // for (int i=0 ; i <= 1 ; i++)
          // {
          //   analogWrite(haptic, 153);
          //   delay(100);
          //   analogWrite(haptic,0);
          //   delay(100);
          // }
          // analogWrite(haptic, 153);
          // delay(200);
          // analogWrite(haptic,0);
          // delay(100);
        }
        else if ((transmissionType == "R")&&(actionCode_2 == "M")&&(roundNumber.toDouble() == (lastRound+1)))
        {
          action = 2;
          monsterRound = roundNumber.toDouble();
          // for (int i=0 ; i <= 1 ; i++)
          // {
          //   analogWrite(haptic, 153);
          //   delay(100);
          //   analogWrite(haptic,0);
          //   delay(100);
          // }
          // analogWrite(haptic, 153);
          // delay(200);
          // analogWrite(haptic,0);
          // delay(100);
        }
        else if ((transmissionType == "R")&&(actionCode_2 == "S")&&(roundNumber.toDouble() == (lastRound+1)))
        {
          action = 3;
          monsterRound = roundNumber.toDouble();
          // for (int i=0 ; i <= 1 ; i++)
          // {
          //   analogWrite(haptic, 153);
          //   delay(100);
          //   analogWrite(haptic,0);
          //   delay(100);
          // }
          // analogWrite(haptic, 153);
          // delay(200);
          // analogWrite(haptic,0);
          // delay(100);            
        }
      }
    }
  }
}

void displayGameStartup(){
  display.display();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(F("welcome to"));
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(F("SKRIMSLI"));
  display.setTextSize(1);
  display.setCursor(0,25);
  display.print(F("choose your character type"));
  display.display();
}

void displayInfo(String line1, String line2, String line3){           // send words to screen for display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(line1);
  display.setCursor(0, 10);
  display.print(line2);
  display.setCursor(0,20);
  display.print(line3);
  display.display();
}

String getValue(String data, char separator, int index)       // parse data
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