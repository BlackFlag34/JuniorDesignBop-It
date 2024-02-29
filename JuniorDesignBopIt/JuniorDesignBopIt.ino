#include <I2C_8Bit.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Adafruit_PCF8574.h>

#define I2C_ADDR1 0x20  //keys1
#define I2C_ADDR2 0x21  //keys2

int playerSwitch = 3; 
int players = LOW;
int startButton = 4; //********************************************************************************************************************
int row[4] = {21,20,19,18};
int column[4] = {28,27,26,22};
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();
// Your register address, which can be found on your IC's datasheet
//#define DATA_REGISTER_ADDR 0xBB  //general I2C data register, holds the last recieved or the next to send

//declare the start 
bool gameStart = false;
uint8_t roundNum;
uint8_t score;
uint8_t fails;
bool endGame;
bool endRound;
uint8_t killColumn;
uint8_t killRow;
bool lightStates[8][8];
uint8_t emptySpaces;
int playerNum;
int startTime;

bool playerOneDone;
bool playerTwoDone;
bool playerThreeDone;


void setup() {
  Serial.begin(115200); 
  matrix.begin(0x70);
  I2C_8Bit_begin();  //all
  randomSeed(analogRead(A0));
  for(int i = 0; i < 4; i++){
    pinMode(column[i], INPUT_PULLUP);
    pinMode(row[i], OUTPUT);
    digitalWriteFast(row[i], HIGH);
  }
}

void loop() {
  Serial.println("beginning of loop");
  if(!gameStart){
    while(digitalRead(startButton) == LOW){}
    gameInitialize();
  }
  while(!endGame && (score < 99)){
    int failure = newRound();
    if(failure == 0){
      fails = 0;
    }
    fails += failure;
    if(fails >= 2){
      endGame = true;
    }
    Serial.print(score);
    gameStart = false;
  }

  //checkingValues(); 

  Serial.println("end of loop");
  delay(2000);
}

void gameInitialize(){
  players = digitalRead(playerSwitch);
  if(players == LOW) {
    playerNum = 2;
  }else{ 
    playerNum = 3;
  }
  roundNum = 0;
  score = 0;
  fails = 0;
  endGame = false;
  endRound = true;
  emptySpaces = 64;

  Serial.println("initalized");
  gameStart = true;
}



//function to read from the button board durring a round and update the lights
void checkingValues(){
  uint8_t registerData1 = I2C_8Bit_readFromModule(I2C_ADDR1);//RR
  uint8_t registerData2 = I2C_8Bit_readFromModule(I2C_ADDR2);//RR
  Serial.print("Register data 1: ");//RR
  Serial.println(registerData1);//RR

  killColumn = registerData1;
  killRow = registerData2;

  readButtonBoard();
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
    if((killRow & (0x01 << i)) && (killColumn & (0x01 << j))){ 
      if(lightStates[i][j] == true){
        lightStates[i][j] = false;
        emptySpaces++;
        score++;
      }
    }
    matrix.drawPixel(i, j, (lightStates[i][j] ? LED_ON : LED_OFF) );
    }
  }
  matrix.writeDisplay();
}


//function to add monsters at the beginning of rounds
bool addMonsters(){
  if(emptySpaces == 0){return false;}
  //add monsters
  uint8_t monsterNumber = random(roundNum / 2, 10);
  if(monsterNumber > emptySpaces){return false;}
  for(int k = 0; k < monsterNumber; k++){
    if(!placeMonster())
      Serial.println("error, monster cannot be placed");
    }
  return true;
}

//function to place a single monster
bool placeMonster(){
  uint8_t spaceToPlace = random(1, emptySpaces);
  int x = 0;
    for(int i = 0; i < 8; i++){
      for(int j = 0; j < 8; j++){
        if(lightStates[i][j] == false){
          x++;
          if(x == spaceToPlace){
            lightStates[i][j] = true;
            emptySpaces--;
            return true;
          }
        }
      }
    }
  return false;
}

uint8_t newRound(){
  Serial.println("new round");
  endRound = false;
  startTime = millis();
  if(!addMonsters()){endGame = true; return 2;}
  assignRoles();
  while(digitalRead(startButton) == HIGH){}
  while(digitalRead(startButton) == LOW){}
  Serial.println("round Start");
  while((millis() - startTime) < 10000){
    //will check if done********************************************************
    readButtonBoard();
  }
  Serial.println("round over"); 

  if(playersFailed() < 2){endRound = true; return 1;}
  else{endRound = true; return 2;}
}

void assignRoles(){
  return;
}

int playersFailed(){
  return 0;
}

bool playersDone(){
  if(playerNum == 2){
    return true;
  }
  return true;
}



void readButtonBoard(){
  for (int i = 0; i < 4; i++) {
    // Activate current row
    digitalWrite(row[i], LOW); // Set current row LOW to activate

    for (int j = 0; j < 4; j++) {
      // Read the state of the current column
      if (digitalRead(column[j]) == LOW) { // If the column reads LOW, a button is pressed
        Serial.print("Key Pressed at Row: ");
        Serial.print(i);
        Serial.print(", Column: ");
        Serial.println(j);
        // Add a small delay to debounce
        delay(50);
      }
    }

    // Deactivate current row before moving to the next
    digitalWrite(row[i], HIGH);
  }
}
