// Comms: Central Game Board
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <IRremote.h>

// #define OLED_MOSI  11         //D1                      // set up OLED screen
// #define OLED_CLK   12         //D0
// #define OLED_DC    9
// #define OLED_CS    8
// #define OLED_RESET 10
// Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// TFT Display Definitions
#define screenPin_DC 9                                            
#define screenPin_CS 10
#define screenPin_BL 12
#define screenPin_RST 8
Adafruit_GC9A01A tft(screenPin_CS, screenPin_DC, screenPin_RST);

// IR communication setup and variables (see list @ end of code for information about input IR codes)
#define IRpin 5
int IRinputValue;

// LoRa as SoftwareSerial
SoftwareSerial lora(3,2);

//Ty's pins
int specialVal0 = A0;
int specialVal1 = A1;
int assignBool = A2;

int roundFlag = A3; 
int startGameFlag = 7;

int readyRound = A5;

int specialDone = 4;


// indicators: startup, active, special_1, special_2, special_3   ;   startup also indicates sensor reading
int specialLight_1 = 6, specialLight_2 = A6, specialLight_3 = A7;

// special character information (characterSelect = 1, 2, 3 => Knight, Mage, Archer (iterates per gauntlet))
int characterSelect_1, characterSelect_2, characterSelect_3;

// gauntlet attack information (attackType = 1, 2, 3 => Kill Monster, Move Monster, Special Attack)
// int attackType_1, attackType_2, attackType_3;
// actually, it will throw things off if I do this now so GAT => gauntlet attack transmission information (iterates per gauntlet)
String GAT1, GAT2,  GAT3;

// truth for holds
bool specialSelect_1 = false, specialSelect_2 = false, specialSelect_3 = false, specialAttackPerformed = false, attackInformationDefined = false;

// round identifier
double monsterRound = 0;



void setup() {
  
  pinMode(specialVal0, INPUT);
  pinMode(specialVal1, INPUT);
  pinMode(assignBool, INPUT);
  pinMode(startGameFlag, OUTPUT);
  pinMode(readyRound, INPUT);
  pinMode(specialDone, OUTPUT);

  digitalWrite(specialDone, LOW);
  digitalWrite(startGameFlag, LOW);
  
  Serial.begin(115200);
  lora.begin(115200);
  IrReceiver.begin(IRpin, DISABLE_LED_FEEDBACK);
  tft.begin();
  pinMode(screenPin_BL, OUTPUT);  digitalWrite(screenPin_BL, HIGH);   // screen backlight on

  randomSeed(35);

  pinMode(3, INPUT);    pinMode(2, OUTPUT);  // LoRa pin definitions


  displayGameStartup();

  while((specialSelect_1==false)||(specialSelect_2==false)||(specialSelect_3==false)){  getCharacterTypes(); }  
  IRinputValue = 50;
  digitalWrite(startGameFlag, HIGH);

  for (int i=0 ; i<125 ; i++){  sendTransmission("CC");   delay(85);  }     // send character selection confirm to all gauntlets   

  displayGameBegin();
}

bool special_preformed = false;

void loop() 
{
  special_preformed = false;
  while (digitalRead(readyRound) == LOW){}
  while (attackInformationDefined == false){ setAttacks(); }
  while (digitalRead(roundFlag) == LOW){}
  while (digitalRead(roundFlag) == HIGH){
    while (attackInformationDefined == true){ playRound();  }
  }
}

void getCharacterTypes()
{
  Serial.println(F("Receiving IR..."));
  receiveIR();
  Serial.println(F("IR Received:"));
  Serial.println(IRinputValue);
  if (IRinputValue == 27){characterSelect_1 = 0; specialSelect_1 = true;}
  if (IRinputValue == 1){characterSelect_1 = 1; specialSelect_1 = true;}
  if (IRinputValue == 2){characterSelect_1 = 2; specialSelect_1 = true;}

  if (IRinputValue == 9){characterSelect_2 = 0; specialSelect_2 = true;}
  if (IRinputValue == 10){characterSelect_2 = 1; specialSelect_2 = true;}
  if (IRinputValue == 11){characterSelect_2 = 2; specialSelect_2 = true;}

  if (IRinputValue == 18){characterSelect_3 = 0; specialSelect_3 = true;}
  if (IRinputValue == 19){characterSelect_3 = 1; specialSelect_3 = true;}
  if (IRinputValue == 20){characterSelect_3 = 2; specialSelect_3 = true;}

  if (specialSelect_1 == true){digitalWrite(specialLight_1, HIGH);}
  if (specialSelect_2 == true){digitalWrite(specialLight_2, HIGH);}
  if (specialSelect_3 == true){digitalWrite(specialLight_3, HIGH);}

  if ((specialLight_1 == HIGH)&&(specialLight_2 == HIGH)&&(specialLight_1)){
    digitalWrite(specialLight_1, LOW);
    digitalWrite(specialLight_2, LOW);
    digitalWrite(specialLight_3, LOW);}
}

void receiveIR()
{
  if (IrReceiver.decode()) 
  {
    IrReceiver.resume();
    IRinputValue = IrReceiver.decodedIRData.command;   
    IrReceiver.resume();
  } 
}

void sendTransmission(String transmissionType)
{
  String info;
  info = info + ",B" + "%" + transmissionType + "%" + String(monsterRound) + "%" + GAT1 + "%" + GAT2 + "%" + GAT3;
  String info_length = String(info.length());
  lora.println("AT+SEND=0,"+info_length+info+"\r");

  Serial.println(F("LoRa transmission sent:"));
  Serial.println("AT+SEND=0,"+info_length+info+"\r");
  Serial.println("");
}

void playRound()
{
  // // send round instructions to all gauntlets  ("R" stands for "round" and includes info from list @ end of code)
  // for (int i = 0 ; i < 155 ; i++){  sendTransmission("R");  delay(85);  }                         
  
  IRinputValue = 50;

  // SAI == special attack information (using input IR codes from list @ end of code)
  int SAI = 0;
  while ((SAI < 3)||(SAI > 27)||(SAI == 18)||(SAI == 19)||(SAI == 20)||(SAI == 9)||(SAI == 10)||(SAI == 11)){ receiveIR();  SAI = IRinputValue; }

  if ((GAT1 == "S") && ((characterSelect_1 == 0 && SAI == 3)  || (characterSelect_1 == 1 && SAI == 5)  || (characterSelect_1 == 2 && SAI == 7))) 
    { specialAttackPerformed = true; attackInformationDefined = false; }
  else if ((GAT2 == "S") && ((characterSelect_2 == 0 && SAI == 12) || (characterSelect_2 == 1 && SAI == 14) || (characterSelect_2 == 2 && SAI == 16)))
    { specialAttackPerformed = true; attackInformationDefined = false; }
  else if ((GAT3 == "S") && ((characterSelect_3 == 0 && SAI == 21) || (characterSelect_3 == 1 && SAI == 23) || (characterSelect_3 == 2 && SAI == 25)))
    { specialAttackPerformed = true; attackInformationDefined = false; }
  else{ specialAttackPerformed = false; attackInformationDefined = false;}

  if (specialAttackPerformed == true)
  {
    digitalWrite(specialDone, HIGH);
  }
}

void setAttacks(){
  digitalWrite(specialDone, LOW);
  int specialVal;

  if((digitalRead(specialVal0) == LOW) && (digitalRead(specialVal1) == LOW)){
    specialVal = 0;
  }else if((digitalRead(specialVal0) == HIGH) && (digitalRead(specialVal1) == LOW)){
    specialVal = 1;
  }else{
    specialVal = 2;
  }

  int randomActions;

  if(digitalRead(assignBool) == LOW){
    randomActions = 0;
  }else{
    randomActions  = 1;
  }

  if (specialVal == 0)
  {
    monsterRound++;
    GAT1 = "S";
    if (randomActions == 0){      GAT2 = "K";   GAT3 = "M";}
    else{ GAT2 = "M";   GAT3 = "K";}
    attackInformationDefined = true;
  }

  if (specialVal == 1)
  {
    monsterRound++;
    GAT2 = "S";
    if (randomActions == 0){      GAT1 = "K";   GAT3 = "M";}
    else{ GAT1 = "M";   GAT3 = "K";}
    attackInformationDefined = true;
  }  

  if (specialVal == 2)
  {
    monsterRound++;
    GAT3 = "S";
    if (randomActions == 0){      GAT1 = "K";   GAT2 = "M";}
    else{ GAT1 = "M";   GAT2 = "K";}
    attackInformationDefined = true;
  }  

  Serial.println(F("Round Moves Defined:"));
  Serial.println(F("Round Number:"));
  Serial.println(monsterRound);
  Serial.println("Gauntlet 1 Instructions:");
  Serial.println(GAT1);
  Serial.println("Gauntlet 2 Instructions:");
  Serial.println(GAT2);
  Serial.println("Gauntlet 3 Instructions:");
  Serial.println(GAT3);
  Serial.println("");

  // send round instructions to all gauntlets  ("R" stands for "round" and includes info from list @ end of code)
  for (int i = 0 ; i < 155 ; i++){  sendTransmission("R");  delay(85);  }   
}

void displayGameStartup(){
  tft.setRotation(3);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(70, 30);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(2);
  tft.println(F("Main Board"));
  tft.setCursor(42, 75);
  tft.setTextColor(GC9A01A_BLUE); tft.setTextSize(2);
  tft.println(F("You're Playing"));
  tft.setCursor(30, 95);
  tft.setTextColor(GC9A01A_BLUE); tft.setTextSize(4);
  tft.println(F("SKRIMSLI"));
  tft.setCursor(53, 145);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Choose Your"));
  tft.setCursor(35, 165);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Character Type"));
  tft.println();
  delay(3000);
}

void displayGameBegin(){
  tft.setRotation(3);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(70, 30);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(2);
  tft.println(F("Main Board"));
  tft.setCursor(42, 75);
  tft.setTextColor(GC9A01A_BLUE); tft.setTextSize(2);
  tft.println(F("You're Playing"));
  tft.setCursor(30, 95);
  tft.setTextColor(GC9A01A_BLUE); tft.setTextSize(4);
  tft.println(F("SKRIMSLI"));
  tft.setCursor(53, 145);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Characters"));
  tft.setCursor(65, 165);
  tft.setTextColor(GC9A01A_WHITE);    tft.setTextSize(2);
  tft.println(F("Chosen!"));
  tft.println();
  delay(3000);
}


// IR DECODER
// 
// CHARACTER SELECTIONS
// 0 tells main board that G1 has chosen to be a knight
// 1 tells main board that G1 has chosen to be a mage
// 2 tells main board that G1 has chosen to be an archer
// 9 tells main board that G2 has chosen to be a knight
// 10 tells main board that G2 has chosen to be a mage
// 11 tells main board that G2 has chosen to be an archer
// 18 tells main board that G3 has chosen to be a knight
// 19 tells main board that G3 has chosen to be a mage
// 20 tells main board that G3 has chosen to be an archer
//
// ACTION SUCCESS INFORMATION
// G1
// 3 tells main board that smite has been performed
// 4 tels main board that smite has not been performed
// 5 tells main board that fireball has been performed
// 6 tells main board that fireball has not been performed
// 7 tells main board that arrow has been performed
// 8 tells main board that arrow has not been performed
// G2
// 12 tells main board that smite has been performed
// 13 tels main board that smite has not been performed
// 14 tells main board that fireball has been performed
// 15 tells main board that fireball has not been performed
// 16 tells main board that arrow has been performed
// 17 tells main board that arrow has not been performed
// G3
// 21 tells main board that smite has been performed
// 22 tels main board that smite has not been performed
// 23 tells main board that fireball has been performed
// 24 tells main board that fireball has not been performed
// 25 tells main board that arrow has been performed
// 26 tells main board that arrow has not been performed



// full packet of information from gauntlets to board format:
// info = [gauntlet identifier] + [character type] + [fireball status] + [arrow status] + [smite status] 

// full packet of information from board to gauntlets format:     
// info = [board identifier] + [transmission type] + [round number] + [gauntlet 1 move type] + [gauntlet2 move type] + [gauntlet3 move type]