// central game board Comms
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <IRremote.h>

#define OLED_MOSI  11         //D1                      // set up OLED screen
#define OLED_CLK   12         //D0
#define OLED_DC    9
#define OLED_CS    8
#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// #define DECODE_NEC                                     // setup IR     /    Includes Apple and Onkyo
#define PIN_RECV 4

SoftwareSerial lora(3,2);                              // define LoRa module as SoftwareSerial for communication

int specialLight = A0;
int LED1 = A5, LED2 = A4, LED3 = A3, LED4 = A2, LED5 = A1, LED6 = 5, LED7 = 6, LED8 = 7;
int value; 
int confirm1 = 0;
bool SC1 = false, SC2 = false, SC3 = false, special = false;
int SCt1, SCt2, SCt3;
String GAT1, GAT2,  GAT3;
String myString, garbage;

void setup() {
  lora.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC);

  pinMode(3, INPUT);
  pinMode(2, OUTPUT);
  pinMode(specialLight, OUTPUT);
  pinMode(LED1, OUTPUT);    pinMode(LED2, OUTPUT);    pinMode(LED3, OUTPUT);    pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);    pinMode(LED6, OUTPUT);    pinMode(LED7, OUTPUT);    pinMode(LED8, OUTPUT);
  digitalWrite(specialLight, LOW); 
  digitalWrite(LED1, HIGH); digitalWrite(LED2, LOW);  digitalWrite(LED3, LOW);  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);  digitalWrite(LED6, LOW);  digitalWrite(LED7, LOW);  digitalWrite(LED8, LOW);

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

  IrReceiver.begin(PIN_RECV, DISABLE_LED_FEEDBACK);
  while((SC1==false)||(SC2==false)||(SC3==false))     // all characters must have a designated type to continue past this point in the code
  {
    getCharacterTypes();
  }  
  value = 50;

  delay(500);   digitalWrite(LED1, LOW);   digitalWrite(LED6, HIGH);
  delay(1000);  digitalWrite(LED6, LOW);   digitalWrite(LED7, HIGH);
  delay(1000);  digitalWrite(LED7, LOW);   digitalWrite(LED8, HIGH);
  delay(1000);  digitalWrite(LED8, LOW);   digitalWrite(LED2, HIGH);

  sendTransmission(F("CC"));
}

void loop() 
{
  // determine move type
  if (monsterRound <= 99)
  {
    playRound();
  }
}

void getCharacterTypes()
{
  receiveIR();

  switch(value){
    case 27 : SCt1 = 0; SC1 = true;
    case 1  : SCt1 = 1; SC1 = true;
    case 2  : SCt1 = 2; SC1 = true;

    case 9  : SCt2 = 0; SC2 = true;
    case 10 : SCt2 = 1; SC2 = true;
    case 11 : SCt2 = 2; SC2 = true;

    case 18 : SCt3 = 0; SC3 = true;
    case 19 : SCt3 = 1; SC3 = true;
    case 20 : SCt3 = 2; SC3 = true;
    default : break;
  }

  if (SC1 == true){ digitalWrite(LED6, HIGH);}
  if (SC2 == true){ digitalWrite(LED7, HIGH);}
  if (SC3 == true){ digitalWrite(LED8, HIGH);}
}

void receiveIR()
{
  if (IrReceiver.decode()) 
  {
    IrReceiver.resume();
    value = IrReceiver.decodedIRData.command;
    display.clearDisplay();
    display.setTextSize(1);                             // display is just for prototyping ; main board won't have a display
    display.setCursor(0,0);
    display.print(F("received value: "));
    display.setCursor(0, 10);
    display.print(value);
    display.setCursor(0, 20);
    display.print("successfully parsed");
    display.display();
    display.clearDisplay();       
    IrReceiver.resume();                                // Important, enables to receive the next IR signal
  } 
}

void sendTransmission(String transmissionType)
{ 
  displayInfo(F("Sending... "),"");
  digitalWrite(LED6, HIGH);
  delay(1000);
  digitalWrite(LED6, LOW);

  String info = "B" + "%" + transmissionType + "%" + String(monsterRound) + "%" + GAT1 + "%" + GAT2 + "%" + GAT3;

  String mymessage = "AT+SEND=0,"+String(info.length())+","+info+"\r";

  displayInfo(F("MSG: "), mymessage);
  delay(1000);
  digitalWrite(LED7, HIGH);
  delay(1000);
  digitalWrite(LED7, LOW);
  for (int i = 0 ; i <= 1000 ; i++)
  {
    lora.println(mymessage);
    delay(10);
    digitalWrite(LED8, HIGH);
    delay(50);
    digitalWrite(LED8, LOW);      
  }
  info = "";
}

void playRound()
{
  monsterRound++;
  long special = random(3);
  if (special == 0){
    GAT1 = "S";                                                             // GAT = gauntlet action transmission definitions
    long randomActions = random(2);
    if (randomActions == 0){      GAT2 = "K";   GAT3 = "M";}
    else if (randomActions == 1){ GAT2 = "M";   GAT3 = "K";}
  }

  else if (special == 1){
    GAT2 = "S";
    long randomActions = random(2);
    if (randomActions == 0){      GAT1 = "K";   GAT3 = "M";}
    else if (randomActions == 1){ GAT1 = "M";   GAT3 = "K";}
  }

  else if (special == 2){
    GAT3 = "S";
    long randomActions = random(2);
    if (randomActions == 0){      GAT1 = "K";   GAT2 = "M";}
    else if (randomActions == 1){ GAT1 = "M";   GAT2 = "K";}
  }

  sendTransmission("R");                                                  // send round instructions to all gauntlets
  int SAI = value;                                                        // define special attack information
  while ((SAI < 0)||(SAI > 26)){  receiveIR();  SAI = value; }

  if ((GAT1 == "S") && ((SCt1 == 0 && SAI == 3)  || (SCt1 == 1 && SAI == 5)  || (SCt1 == 2 && SAI == 7))) {
       special = true; }
  }
  if ((GAT2 == "S") && ((SCt2 == 0 && SAI == 12) || (SCt2 == 1 && SAI == 14) || (SCt2 == 2 && SAI == 16))){ 
      special = true; }
  }  
  if ((GAT3 == "S") && ((SCt3 == 0 && SAI == 21) || (SCt3 == 1 && SAI == 23) || (SCt3 == 2 && SAI == 25))){ 
      special = true; }
  }
  if (special == true)
  {
    digitalWrite(specialLight, HIGH);
    delay(3000); 
  }
  digitalWrite(specialLight, LOW);
  special = false;
}

void displayInfo(String identifier, String message){                       // send words to screen for display
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(identifier);
  display.setCursor(0, 10);
  display.print(message);
  display.display();
  display.clearDisplay();
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