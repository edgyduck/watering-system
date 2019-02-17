/* last change 2019 Feb 17th
    Changes:
    - Adjusted moistureLowLimit, check time, water pumping interval
    - Added RTC time adjustment in setup
*/
int waterSensor = 9;  // simple float switch. Water low = LOW (1), water high = HIGH (0)
int moistureSensor = A3;
int waterPump = 13;
int buzzer = 7;
int servoSig = 9;
int lcdContrastPin = A2;
int lcdContrastLvl = 50;

#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;
char day[4];
char month[20];
char time[30];

#include <LiquidCrystal.h>
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int lcdBrightnessPin = 8; // to control lcd brightness
int lcdTurnOnHour = 9;  // turn on lcd backlight at 8 AM and turn off at 10 PM
int lcdTurnOffHour = 22;

int moistureLowLimit = 5; // Set low limit (percent) for moisture and water sensors
int waterLowLimit = 0; // now using flaot switch so either 1 or 0
int checkHour = 11;   // Run moisture check at 17:15 each day
int checkMinute = 30;
int lastCheckDay = 0; // Mark last check day and month
int lastCheckMonth = 0;
  
void setup() {
//   put your setup code here, to run once:
  //Serial.begin(9600);
  pinMode(lcdContrastPin, OUTPUT);
  analogWrite(lcdContrastPin, lcdContrastLvl);
  pinMode(lcdBrightnessPin, OUTPUT);
  digitalWrite(lcdBrightnessPin, HIGH);
  delay(500);
  pinMode(waterSensor, INPUT_PULLUP);
  pinMode(moistureSensor, INPUT);
  pinMode(waterPump, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH); // indicate power on
  delay(50);
  digitalWrite(buzzer, LOW);
  delay(50);
  
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    //Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  lcd.begin(16,2);
  
}
 
void loop() {
  // Check if it's time to measure moisture and water
  DateTime now = RTC.now();
  int currMonth = now.month();
  int currDay = now.day();
  int currHour = now.hour();
  int currMinute = now.minute();
  sprintf( time, "%02hhu:%02hhu", now.hour(), now.minute() );
  //Serial.print("Starting time: ");
  //Serial.println(time);

  if (currHour == lcdTurnOnHour)
  {
    digitalWrite(lcdBrightnessPin, HIGH);
  }
  if (currHour == lcdTurnOffHour)
  {
    digitalWrite(lcdBrightnessPin, LOW);
  }

 if(currDay - lastCheckDay != 0 && strcmp(time, "11:30") == 0) // if it's not the same day and time is 17:15 then do the job
 //if(strcmp(time, "17:30") != 0) // condition for testing purposes...
  {        
    lcd.setCursor(0, 1);
    lcd.print("Laikas laistyti!");
    delay(1000);
    int moistureLevel = getMoistureLevel(moistureSensor, currHour, currMinute);
    if (moistureLevel < moistureLowLimit) // If moisture is below limit
    {
        int waterLevel = getWaterLevel(waterSensor); // Check water level in the tank:
        if (waterLevel > waterLowLimit) // If there is enough water
        {
          waterPlants(waterPump, currMonth, currDay); // Start watering
        }
        else
        {
          while (waterLevel == waterLowLimit)   // while there is not enough water in the tank
          {
            for (int i = 0; i < 3; i++)
            {
              digitalWrite(buzzer, HIGH); // indicating low water
              delay(100);
              digitalWrite(buzzer, LOW);
              delay(50);
            }
            lcd.setCursor(0,1);
            lcd.print("NERA VANDENS!!!!");  
            delay(5000);
            waterLevel = getWaterLevel(waterSensor);
          }
          waterPlants(waterPump, currMonth, currDay);
        }
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("Dregmes gana... ");
      delay(3000);
    }
  lastCheckDay = currDay; // Mark today's day as last check day
  }
  else
  {
    // do moisture checks every 15 seconds
    //lcd.setCursor(0, 1);
    //lcd.print("Stebiu dregme...");
    //delay(3000);
    getMoistureLevel(moistureSensor, currHour, currMinute);
    delay(15000);
  }
    
}

int getMoistureLevel(int sensorPin, int currHour, int currMinute)
{
  lcd.setCursor(0,0); 
  lcd.print("Tikrinu dregme...");
  delay(1000);

  int moistureRAW = abs(analogRead(sensorPin));
  int moisture = constrain(moistureRAW, 2, 660);   // accept values between these limits for 4.8V on sensor 
  int moisturePercent = map(moisture, 2, 660, 0, 100); // and map them between 0 and 100%
  //Serial.print("Moisture level: ");
  //Serial.println(moisturePercent);

  lcd.setCursor(0,0);
  lcd.print("Dregme ");
  lcd.print(moisturePercent);
  lcd.print("% ");
  lcd.print(currHour);
  lcd.print(":");
  lcd.print(currMinute);
  lcd.print("  ");
  delay(1000);
  return moisturePercent;
}

int getWaterLevel(int sensorPin)
{
  lcd.setCursor(0, 1);
  lcd.print("Tikrinu vandeni..");
  delay(2000);
  
//  int waterLvlRAW = analogRead(sensorPin);
//  //Serial.print("Water RAW: ");
//  //Serial.println(waterLvlRAW);
//  int waterLvl = constrain(waterLvlRAW, 280, 730);
//  int waterLvlPercent = map(waterLvl, 280, 730, 0, 100); // and map them between 0 and 100%
//  //Serial.print("Water level: ");
//  //Serial.println(waterLvlPercent);

  lcd.setCursor(0,1);
  lcd.print("Vandens: ");

  int waterLvl; // marker for water level
  if (digitalRead(sensorPin) == LOW)
  {
    waterLvl = 0; // Low water level, needs refill
    lcd.print("yra");
  }
  else
  {
    waterLvl = 1; // Water level OK
    lcd.print("NERA!");
  }

  delay(1000);
  return waterLvl;
}

void waterPlants(int pumpPin, int currMonth, int currDay)
{
  
  lcd.setCursor(0,1);
  lcd.print("Laistau.....");
  delay(1000);

  digitalWrite(pumpPin, HIGH);
  delay(20000);
  digitalWrite(pumpPin, LOW);
  delay(2000);

  lcd.setCursor(0,1);
  lcd.print("Laisciau: ");
  lcd.print(currMonth);
  lcd.print("m");
  lcd.print(currDay);
  lcd.print("d");
  delay(5000);
}
