#include <I2C_8Bit.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Adafruit_PCF8574.h>
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>

#define TFT_DC 8 //pin
#define TFT_CS 1 //cs
#define TFT_MOSI 3 //tx
#define TFT_CLK 2 //sck
#define TFT_RST 6 //pin
#define TFT_MISO 0 //rx

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Pause in milliseconds between screens, change to 0 to time font rendering
#define WAIT 3000
#define FONT_SIZE 4

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


int players = LOW;
int startButton = 7;

int specialVal0 = 13;
int specialVal1 = 14;
int assignBool = 12;

int roundFlag = 11; 
int startGameFlag = 10;

int readyRound = 9;

int specialDone = 15;

Adafruit_PCF8574 columns;
Adafruit_PCF8574 killRows;
Adafruit_PCF8574 moveRows;
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

//declare the start 
bool gameStart = false;
long roundNum;
uint8_t score;
uint8_t fails;
bool endGame;
bool endRound;

bool lightStates[5][5];
long emptySpaces;
int playerNum;
int startTime;

bool killPlayerDone;
bool movePlayerDone;
bool specialPlayerDone;

bool firstMove;

int movedMonsterI;
int movedMonsterJ;

String oldPrint;

void setup() {
  Serial.begin(115200); 
  matrix.begin(0x70);
  columns.begin(0x20, &Wire);
  killRows.begin(0x22, &Wire);
  moveRows.begin(0x21, &Wire);
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
   tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextSize(1);

  for(int i = 0; i < 5; i++){
    columns.pinMode(i, INPUT_PULLUP);
    killRows.pinMode(i, OUTPUT);
    killRows.digitalWrite(i, HIGH);
    moveRows.pinMode(i, OUTPUT);
    moveRows.digitalWrite(i, HIGH);
    for(int j = 0; j < 5; j++){
      lightStates[i][j] = false;
    }
  }
  pinMode(specialVal0, OUTPUT);
  pinMode(specialVal1, OUTPUT);
  pinMode(assignBool, OUTPUT);
  pinMode(roundFlag, OUTPUT);
  pinMode(startButton, INPUT);
  pinMode(startGameFlag, INPUT);
  pinMode(readyRound, OUTPUT);
  pinMode(specialDone, INPUT);

  digitalWrite(specialVal0, LOW);
  digitalWrite(specialVal1, LOW);
  digitalWrite(assignBool, LOW);
  digitalWrite(roundFlag, LOW);
  digitalWrite(readyRound, LOW);

  killPlayerDone = false;
  movePlayerDone = false;
  specialPlayerDone = false;
}



void clear(){
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0);
}
void loop() {
  randomSeed(millis());
  if(!gameStart){
    clear();
    tft.println("Waiting to start");
    while(digitalRead(startGameFlag) == LOW){} 
    Serial.println("Game Initializing");
    gameInitialize();
  }
  else{
    while(!endGame && (score < 99)){
      clear();
      tft.println("New Round\ngetting ready ...");
      int failure = newRound();
      tft.println("Round Over!");
      delay(WAIT);
      if(failure == 0){
        fails = 0;
      }
      fails += failure;
      if(fails >= 2){
        endGame = true;
        clear();
        tft.println("Game Over");
        tft.println("Score is " + String(score));
        gameStart = false;
        while(digitalRead(startButton) == LOW){}
        while(digitalRead(startButton) == HIGH){}
      }
    }
    clear();
        tft.println("Game Over");
        tft.println("Score is " + String(score));
  }
}

void gameInitialize(){

  roundNum = 0;
  score = 0;
  fails = 0;
  endGame = false;
  endRound = true;
  emptySpaces = 25;
  matrix.clear();
  matrix.writeDisplay();
  for(int i = 0; i < 5; i++){
    for(int j = 0; j < 5; j++){
      lightStates[i][j] = false;
    }
  }

  gameStart = true;

  Serial.println("Game Initialized");
}



//function to read from the button board durring a round and update the lights
void checkingValues(){
  readButtonBoard();
  if(digitalRead(specialDone) == HIGH){
    specialPlayerDone = true;
  }
}


//function to add monsters at the beginning of rounds
bool addMonsters(long monsterNumber){
  if(emptySpaces == 0){return false;}
  for(int k = 0; k < monsterNumber; k++){
    if(!placeMonster()){
      Serial.println("error, monster cannot be placed");
      return false;
      }
    }
  return true;
}

//function to place a single monster
bool placeMonster(){
  if(emptySpaces == 0){return false;}
  long spaceToPlace = random(1, emptySpaces);
  int x = 0;
    for(int i = 0; i < 5; i++){
      for(int j = 0; j < 5; j++){
        if(lightStates[i][j] == false){
          x++;
          if(x == spaceToPlace){
            lightStates[i][j] = true;
            matrix.drawPixel(i, j, LED_ON);
            matrix.writeDisplay();
            emptySpaces--;
            return true;
          }
        }
      }
    }
    Serial.println("error, no place to place");
  return false;
}

uint8_t newRound(){
  roundNum++;
  Serial.println("new round");
  endRound = false;
  randomSeed(millis());
  //add monsters
  long monsterNumber;
  if(roundNum < 7){
    monsterNumber = random(roundNum, 7);
  }
  else{
    monsterNumber = random(6, 8);
  }
  Serial.println("random number of monsters = " + String(monsterNumber));
  if(monsterNumber >= emptySpaces){endGame = true; return 2;}

  assignRoles();
  tft.println("Round Ready");

  digitalWrite(readyRound, HIGH);
  while(digitalRead(startButton) == HIGH){}
  while(digitalRead(startButton) == LOW){}
  digitalWrite(readyRound, LOW);
  digitalWrite(roundFlag, HIGH);
  killPlayerDone = false;
  movePlayerDone = false;
  specialPlayerDone = false;
  firstMove = false;
  addMonsters(monsterNumber);
  clear();
  int x = 10;   
  tft.println("round Start");
  startTime = millis();
  while((millis() - startTime) < 10000){
    if((millis() - startTime) > ((10 - x) * 1000)){
      tft.println(x);
      --x;
    }
    checkingValues();
    if(killPlayerDone && movePlayerDone && specialPlayerDone){
      endRound = true;
      Serial.println("round finished"); 
      digitalWrite(roundFlag, LOW);
      return 0;
    }
  }
  Serial.println("round over"); 
  digitalWrite(roundFlag, LOW);
  
 endRound = true; 
 return playersFailed();
}

void assignRoles(){
  randomSeed(millis());
  long special = random(0, 3);
  long others = random(0,2);
  Serial.println(String(special));
  if(special == 2){
    digitalWrite(specialVal0, LOW);
    digitalWrite(specialVal1, LOW);
    Serial.println("special 2");
  }else if(special == 1){
    digitalWrite(specialVal0, HIGH);
    digitalWrite(specialVal1, LOW);
    Serial.println("special 1");
  }else{
    digitalWrite(specialVal0, LOW);
    digitalWrite(specialVal1, HIGH);
    Serial.println("special 0");
  }

  Serial.println(String(others));
  if(others == 1){
    digitalWrite(assignBool, HIGH);
    Serial.println("assign 1");
  }else{
    digitalWrite(assignBool, LOW);
    Serial.println("assign 0");
  }
  
  return;
}

int playersFailed(){
  int fails = 0;
  if(!killPlayerDone){fails++;}
  if(!movePlayerDone){
    fails++;
    if(firstMove){
      lightStates[movedMonsterI][movedMonsterJ] = true;
      matrix.writePixel(movedMonsterI, movedMonsterJ, LED_ON);
      matrix.writeDisplay();
    }
  }
  if(!specialPlayerDone){fails++;}
  return fails;
}

void readButtonBoard(){
  for (int i = 0; i < 5; i++) {
    // Activate current row
     killRows.digitalWrite(i, LOW); // Set current row LOW to activate
    for (int j = 0; j < 5; j++) {
      // Read the state of the current column
      if (columns.digitalRead(j) == LOW) { // If the column reads LOW, a button is pressed
        // Add a small delay to debounce
        delay(50);
        if(lightStates[i][j] == true){
          lightStates[i][j] = false;
          matrix.drawPixel(i,j, LED_OFF);
          matrix.writeDisplay();
          emptySpaces++;
          score++;
          killPlayerDone = true;
        }
      }
    }

    // Deactivate current row before moving to the next
    killRows.digitalWrite(i, HIGH);
  }
  for(int i = 0; i < 5; i++){
    moveRows.digitalWrite(i, LOW);
    for(int j = 0; j < 5; j++) {
      if(columns.digitalRead(j) == LOW) {
        delay(50);
        if(movePlayerDone == true){}
        else if((lightStates[i][j] == true) && (firstMove == false)){
          firstMove = true;
          movedMonsterI = i;
          movedMonsterJ = j;
          lightStates[i][j] = false;
          matrix.drawPixel(i, j, LED_OFF);
          matrix.writeDisplay();
        }
        else if((lightStates[i][j] == false) && (firstMove == true) && ((movedMonsterI == i) && (movedMonsterJ == j))){}
        else if((lightStates[i][j] == false) && (firstMove == true)){
          movePlayerDone = true;
          lightStates[i][j] = true;
          matrix.drawPixel(i, j, LED_ON);
          matrix.writeDisplay();
        }
      }
    }
    // Deactivate current row before moving to the next
    moveRows.digitalWrite(i, HIGH);
  }
}


