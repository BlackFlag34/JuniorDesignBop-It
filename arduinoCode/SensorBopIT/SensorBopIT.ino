#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_Sensor.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include "mp3tf16p.h"

MP3Player mp3(10,11);

#define ArrowDraw 1
#define FireBall 2
#define SwordSmite 3
#define ArrowLaunch 4

int irC = 0, sF = 0, aF = 0;
MPU6050 mpu6050(Wire);

void magicCast(void)
{
  Serial.println("MAGIC!");
  while(irC < 4)
  {
    digitalWrite(2, HIGH);
  
    if (analogRead(A3) < 600 && irC == 0)
    {
      // digitalWrite(14, HIGH);
      Serial.println("A3 Read");
      irC++;
    }
  
    if (analogRead(A2) < 600 && irC == 1)
    {
      // digitalWrite(16, HIGH);
      Serial.println("A2 Read");
      irC++;
    }
  
    if (analogRead(A1) < 600 && irC == 2)
    {
      // digitalWrite(10, HIGH);
      Serial.println("A1 Read");
      irC++;
    }
  
    if (analogRead(A0) < 600 && irC == 3)
    {
      mp3.playTrackNumber(FireBall, 25, false);
      digitalWrite(8, HIGH);
      delay(5000);
      Serial.println("A0 Read");
      irC = 5;
    }
  }

  irC = 0;
}

void smiteSword(void)
{
  byte smite = 0;
  smite = 52;
  Serial.println("SWORD!");
  while (sF == 0)
  {
    mpu6050.update();
    Serial.println(mpu6050.getAccZ());
    Serial.println("");
    if (mpu6050.getAccZ() > 1.90)
    {
      digitalWrite(8, HIGH);
      mp3.playTrackNumber(SwordSmite, 25, false);
      sF = 1;
      Serial.write(smite);
    }
  }

  sF = 0;
}

void drawArrow(void)
{
  // Serial.println("ARROW!");
  while (aF == 0)
  {
    if (digitalRead(7) == HIGH)
    {
      mp3.playTrackNumber(ArrowDraw, 25);
      while(digitalRead(7) == HIGH);
      mp3.playTrackNumber(ArrowLaunch, 25);
      digitalWrite(8, HIGH);
      delay(3000);
      aF = 1;
      byte arrow = 0;
      arrow = 52;

      Serial.write(arrow);
      digitalWrite(8, LOW);
    }
  }
  aF = 0;
}

void setup() 
{
  pinMode(2, OUTPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, OUTPUT);
  // pinMode(10, OUTPUT);
  // pinMode(14, OUTPUT);
  // pinMode(16, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  digitalWrite(8,  LOW);
  Serial.begin(115200);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  mp3.initialize();
}

void loop() 
{
  byte data = 0;
  if (Serial.available() > 0)
  {
    int x = Serial.read();
    data = x;
    Serial.println(data);
    if (data == 25)
      drawArrow();
  }
}
    
  
