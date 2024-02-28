// gauntlet #1

#include <SPI.h>                                             // include libraries
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <IRremote.h>

#define OLED_MOSI  11         //D1                          // set up OLED screen
#define OLED_CLK   12         //D0
#define OLED_DC    9
#define OLED_CS    8
#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#define IRpin       4                                       // setup IR

SoftwareSerial lora(3,2);                                   // define LoRa module as SoftwareSerial for communication

int variableResistor = A0;                                  // potentiometer variables
int vrdata = 0;
int data_length; 

int LED1 = A5;
int LED2 = A4;
int LED3 = A3;
int LED4 = A2;
int LED5 = A1;

int sendButton = 13;
int sensorButton = 5;
bool sent = false;
bool sensor = true;

double monsterRound = 0;

String data;                                                // LoRa communications variables
String myString; 
String garbage;

String characterSelect;
String action;

unsigned long startMillis;                                  // setup timer
unsigned long currentMillis;
const unsigned long period = 3000;

bool gameStart = true;                                      // input selection variable

bool smiteStatus = true;                                     // sensor variables and comm confirm (initialized as false for game but as true for testing)
bool fireballStatus = true;
bool arrowStatus = true;
bool confirm = false;

uint8_t sCommand;                                           // IR variables
uint8_t sRepeats = 3;


void setup() {

  Serial.begin(115200);                                     // begin communications and display
  lora.begin(115200);
  IrSender.begin(IRpin);
  display.begin(SSD1306_SWITCHCAPVCC);

  pinMode(3, INPUT);
  pinMode(2, OUTPUT);
  pinMode(variableResistor,INPUT);                          // inputs
  pinMode(sendButton, INPUT);
  pinMode(sensorButton, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);

  display.display();                                        // starting screen
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

  while (characterSelect==""){ getCharacter(); }           // get character type

  startMillis = millis();                                  // start timer

  while(sent == false)
  {
    if(digitalRead(sendButton))
    {
      sendIR();
      sent = true;
    }
  }
  
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);
}

void loop() 
{
  if (gameStart == true){ receiveLoRa(); }
  else { playRound(); }

  if (sensor == false){ readSensor(); }
}

void getCharacter()
{
  // should have a button confirmation of a choice being made!
  vrdata = map(analogRead(variableResistor),0,1024,0,255);        // read analog data and convert it
  if (vrdata < 75)
  {
    characterSelect = "Knight";
  }
  else if ((vrdata > 75)&&(vrdata < 150)){ characterSelect = "Mage"; }
  else { characterSelect = "Archer"; }
}

void playRound()
{
  if (action == "")                                      // receive action command from game board
  {
    receiveLoRa();
  }
  
  else if (action == "K")
  {
    displayInfo("Round "+String(monsterRound)+" Start", "Your Action: ", "Kill Monster");
    // could display counter, number of monsters, actions for all players, etc
    delay(5000);
  }
  else if (action == "M")
  {
    displayInfo("Round "+String(monsterRound)+" Start", "Your Action: ", "Move Monster");
    // could display counter, number of monsters, actions for all players, etc
    delay(5000);
  }
  else if (action == "S")
  {
    digitalWrite(LED3, HIGH);
    delay(1000);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
    delay(1000);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, HIGH);
    delay(1000);
    digitalWrite(LED5, LOW);
    digitalWrite(LED3, HIGH);  
    delay(1000);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
    delay(1000);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, HIGH);
    delay(1000);
    digitalWrite(LED5, LOW);

    displayInfo("Round "+String(monsterRound)+" Start", "Your Action: ", "SPECIAL POWER");
    delay(5000);    
  }
}

void readSensor()
{
  // read in sensor data
  // really just waits for a button press for now; easier to error-check
  if(digitalRead(sensorButton) == HIGH)
  {
    if (characterSelect == "Knight")
    {
      smiteStatus = true;
    }
    if (characterSelect == "Mage")
    {
      fireballStatus = true;
    }
    if (characterSelect == "Archer")
    {
      arrowStatus = true;
    }
    sendIR();
    sensor = true;
  }
  // may need to add a while loop for confirmation here
}

void sendIR()
{
  if (gameStart == true)
  {
    if (characterSelect == "Knight"){ sCommand = 0x1B; }
    else if (characterSelect == "Mage"){ sCommand = 0x01; }
    else if (characterSelect == "Archer"){ sCommand = 0x02; } 
  }
  else
  {
    if (characterSelect == "Knight")
    {
      if (smiteStatus == true)
      {
        sCommand = 0x03;
      }
      if (smiteStatus == false)
      {
        sCommand = 0x04;
      }
    }
    if (characterSelect == "Mage")
    {
      if (fireballStatus == true)
      {
        sCommand = 0x05;
      }
      if (fireballStatus == false)
      {
        sCommand = 0x06;
      }
    }
    if (characterSelect == "Archer")
    {
      if (arrowStatus == true)
      {
        sCommand = 0x07;
      }
      if (arrowStatus == false)
      {
        sCommand = 0x08;
      }
    }
  }  
  IrSender.sendNEC(0x00, sCommand, sRepeats);
}

void receiveLoRa()                                                    // function to receive and process incoming transmission
{
  if ( lora.available() > 0 )
  {
    // displayInfo("Receiving...","","");
    // delay(1000);
    // garbage = lora.readString();                                      // consists of any errors
    myString = lora.readString();                                     // actual transmission

    displayInfo("Incoming Message: ", myString, "");
    delay(1000);

    String address = getValue(myString, ',', 0);                      // address
    String length = getValue(myString, ',', 1);                       // data length
    String information = getValue(myString, ',', 2);                  // transmission information
    String rssi = getValue(myString, ',', 3);                         //RSSI
    String snr = getValue(myString, ',', 4);                          //SNR

      // full packet of information from board to gauntlets format:     
      // info = [board identifier] + [transmission type] + [round number] + [gauntlet 1 move type] + [gauntlet2 move type] + [gauntlet3 move type]

    String inID = getValue(information, '%', 0);                        // identify sender
    String transmissionType = getValue(information, '%', 1);               
    String roundNumber = getValue(information, '%', 2);
    String GAT1 = getValue(information, '%', 3);    
    String GAT2 = getValue(information, '%', 4);
    String GAT3 = getValue(information, '%', 5);

    if ((inID != "")&&(transmissionType != ""))                           // noise filter
    { 
      if (inID == "B")
      {
        if (transmissionType == "CC")
        {
          displayInfo("Game Start Confirmed", "Character Type: ", characterSelect);
          delay(3000);
          gameStart = false;
          sensor = false;
        }
        if ((transmissionType == "R")&&(GAT1 == "K" || GAT1 == "M" || GAT1 == "S"))   // identify round start with noise filter
        {
          action = GAT1;
          monsterRound = roundNumber.toDouble();
        }
        if (transmissionType == "C") { confirm = true; };
      }
    }       
    myString="";                                                       // clear input string    
  }
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