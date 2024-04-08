#include <I2C_8Bit.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Adafruit_PCF8574.h>
#include "SPI.h"
#include <Adafruit_ILI9341.h>

#define TFT_DC 8
#define TFT_CS 1
#define TFT_MOSI 3
#define TFT_CLK 2
#define TFT_RST 6
#define TFT_MISO 0

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

int players = LOW;
int startButton = 7;

int specialVal0 = 13;
int specialVal1 = 12;
int assignBool = 11;

int roundFlag = 10; 
int startGameFlag = 9;

int readyRound = 8; //to be changed


int specialDone = 14;

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

void setup() {
  Serial.begin(115200); 
  matrix.begin(0x70);
  columns.begin(0x20, &Wire);
  killRows.begin(0x22, &Wire);
  moveRows.begin(0x21, &Wire);
  tft.begin();
  tft.setRotation(2);

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

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Hello World!");
}

void loop() {
  readButtonBoard();

  // randomSeed(millis());
  // if(!gameStart){
  //   while(digitalRead(startGameFlag) == LOW){}
  //   gameInitialize();
  // }
  // while(!endGame && (score < 99)){
  //   int failure = newRound();
  //   if(failure == 0){
  //     fails = 0;
  //   }
  //   fails += failure;
  //   if(fails >= 2){
  //     endGame = true;
  //     Serial.println("Score is " + String(score));
  //     gameStart = false;
  //   }
  // }
}

void gameInitialize(){

  roundNum = 0;
  score = 0;
  fails = 0;
  endGame = false;
  endRound = true;
  emptySpaces = 25;

  gameStart = true;
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
            matrix.drawPixel(j, i, LED_ON);
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
  long monsterNumber = random(roundNum, 10);
  Serial.println("random number of monsters = " + String(monsterNumber));
  if(monsterNumber >= emptySpaces){endGame = true; return 2;}

  assignRoles();

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

  
  startTime = millis();
  Serial.println("round Start");
  while((millis() - startTime) < 10000){
    checkingValues();
    if(killPlayerDone && movePlayerDone && specialPlayerDone){
      endRound = true;
      return 0;
    }
  }
  Serial.println("round over"); 
  digitalWrite(readyRound, LOW);
  
 endRound = true; 
 return playersFailed();
}

void assignRoles(){
  long special = random(0, 2);
  long others = random(0,1);
  Serial.println(String(special));
  if(special == 2){
    digitalWrite(specialVal0, LOW);
    digitalWrite(specialVal1, HIGH);
    Serial.println("special 2");
  }else if(special == 1){
    digitalWrite(specialVal0, HIGH);
    digitalWrite(specialVal1, LOW);
    Serial.println("special 1");
  }else{
    digitalWrite(specialVal0, LOW);
    digitalWrite(specialVal1, LOW);
    Serial.println("special 0");
  }

  Serial.println(String(others));
  if(others == 1){
    digitalWrite(assignBool, HIGH);
    Serial.println("assign 1");
  }else{
    digitalWrite(assignBool, LOW);
    Serial.println("assign 2");
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
      matrix.writePixel(movedMonsterJ, movedMonsterI, LED_ON);
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
        //if(lightStates[i][j] == true){
          // lightStates[i][j] = false;
          Serial.print("( "); Serial.print(i); Serial.print(", ");
          Serial.print(j);  Serial.println(")");
          matrix.drawPixel(i, j, LED_ON);
          matrix.writeDisplay();
          // emptySpaces++;
          // score++;
          // killPlayerDone = true;
       // }
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
      
        // if((lightStates[i][j] == true) && (firstMove == false)){
          // firstMove = true;
          // movedMonsterI = i;
          // movedMonsterJ = j;
          // lightStates[i][j] = false;
          matrix.drawPixel(i, j, LED_OFF);
          matrix.writeDisplay();
        // }
        // else if((lightStates[i][j] == false) && (firstMove == true) && ((movedMonsterI == i) && (movedMonsterJ == j))){
        //   movePlayerDone = true;
        //   lightStates[i][j] = true;
        //   matrix.drawPixel(j, i, LED_ON);
        //   matrix.writeDisplay();
        // }
      }
    }
    moveRows.digitalWrite(i, HIGH);
  }
}
