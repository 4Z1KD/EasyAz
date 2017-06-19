/*
  1. Reads an azimuth from the serial port
  2. Release the brakes
  3. Move the rotator to the position (by reading the on-motor potentiometer)
  4. Lock the brakes
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DisplayService.h"

LiquidCrystal_I2C lcdx(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
DisplayService dispServe(&lcdx);

//Pins setup
const int ROTATE_RIGHT_PIN = 10;
const int ROTATE_LEFT_PIN = 11;
const int BRAKES_PIN = 9;
const int AZIMUTH_PIN = A1;

LiquidCrystal_I2C  lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);  // set the LCD address to 0x27 for a 16 chars and 2 line display
char rawCommand; //The raw string of the command as received in "serialEvent"
int newAzimuth; //holds the parsed azimuth
bool isNewAzimuth = false;
String azimuthReportString = "";
char rotationDir; //holds the rotation direction - being set in "SetRotationDir"

void setup() {
  // this is the place to set general settings and default values //
  dispServe.Init();
  Serial.begin(9600);  // Used to type in characters
  ReportAzimuth();

  //set pins mode and default values
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ROTATE_RIGHT_PIN, OUTPUT);
  digitalWrite(ROTATE_RIGHT_PIN, LOW);
  pinMode(ROTATE_LEFT_PIN, OUTPUT);
  digitalWrite(ROTATE_LEFT_PIN, LOW);
  pinMode(BRAKES_PIN, OUTPUT);
  digitalWrite(BRAKES_PIN, LOW);
  displayWelcomeMsg();//display the welcome message
  delay(1000);//wait for 1 second
  ToggleBrakes(false);//make sure the brakes are locked
}

void loop() {
  if (isNewAzimuth) //if there is a new azimuth
  {
    ToggleBrakes(true);//unlock the brakes
    Rotate();
    ToggleBrakes(false); //lock the brakes
    isNewAzimuth = false; //reset the flag
  }
}

void ToggleBrakes(boolean state)
{
  if (state)//true: activate the rotator (brakes off)
  {
    digitalWrite(BRAKES_PIN, HIGH);//release the brakes
    dispServe.Log("Brakes Off", 0, 0, 1);
  }
  if (!state)
  {
    digitalWrite(BRAKES_PIN, LOW);//lock the brakes
    dispServe.Log("Heading " + String(GetCurrentAzimuth()), 0, 0, 1); //report the new heading
    dispServe.Log("Brakes On", 0, 1, 0);
  }
}

void Rotate()
{
  SetRotationDir(GetCurrentAzimuth(), newAzimuth);
  switch (rotationDir)
  {
    case 'R':
      dispServe.Log("Rotating " + String(GetCurrentAzimuth()) + ">" + String(newAzimuth), 0, 1, 0);
      digitalWrite(ROTATE_RIGHT_PIN, HIGH);
      while (GetCurrentAzimuth() < newAzimuth) {
        ReportAzimuth();
        dispServe.Log("        ", 9, 1, 0);
        dispServe.Log(String(GetCurrentAzimuth()) + ">" + String(newAzimuth), 9, 1, 0);
        delay(1000);
        //if (Serial.available()) break;
      };
      digitalWrite(ROTATE_RIGHT_PIN, LOW);
      break;

    case 'L':
      dispServe.Log("Rotating " + String(GetCurrentAzimuth()) + ">" + String(newAzimuth), 0, 1, 0);
      digitalWrite(ROTATE_LEFT_PIN, HIGH);
      while (GetCurrentAzimuth() > newAzimuth) {
        ReportAzimuth();
        dispServe.Log("        ", 9, 1, 0);
        dispServe.Log(String(GetCurrentAzimuth()) + ">" + String(newAzimuth), 9, 1, 0);
        delay(1000);
        //if (Serial.available()) break;
      };
      digitalWrite(ROTATE_LEFT_PIN, LOW);
      break;

    default:
      break;
  }
}

void SetRotationDir(int from, int to)
{
  if ((to - from) > 0)
  {
    rotationDir = 'R';
  }
  if ((to - from) <= 0)
  {
    rotationDir = 'L';
  }
}

int GetCurrentAzimuth()
{
  int  currentAzimuth = analogRead(AZIMUTH_PIN);
  //return  map(currentAzimuth, 0, 1023, 0, 360);
  return  map(currentAzimuth, 0, 820, 0, 360);
}

void ReportAzimuth()
{
  int currentAzimuth = GetCurrentAzimuth();
  if (currentAzimuth > 99)
    azimuthReportString = "+0" + String(currentAzimuth) ;
  else if (currentAzimuth > 9)
    azimuthReportString = "+00" + String(currentAzimuth) ;
  else if (currentAzimuth >= 0)
    azimuthReportString = "+000" + String(currentAzimuth) ;
  Serial.print(azimuthReportString);
  delay(50);
  Serial.print(azimuthReportString);
  delay(50);
  Serial.print(azimuthReportString);
  Serial.print("+0100");
}

void displayWelcomeMsg()
{
  dispServe.Blink(3);
  dispServe.Log11("EzAz Rotator", 0, 0, 1, 50);
  dispServe.Log11("By 4Z1KD", 0, 1, 0, 200);
}

void serialEvent() {
  while (Serial.available())
  {
    rawCommand = Serial.read(); // Read a character
    if (rawCommand == 'C') //Prog is asking what is the rotator azimuth
    {
      ReportAzimuth();
    }
    if (rawCommand == 'M') //The prog sent new azimuth
    {
      newAzimuth = (Serial.parseInt()); //get the wanted Az from the string
      isNewAzimuth = true;
    }
  }
}

void Flash13(int repeats)
{
  for (int i = 0; i < repeats; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}

