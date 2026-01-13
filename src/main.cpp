#include <Neza74HC165.h>
#include <Adafruit_NeoPixel.h>
#include <vector>
#include <Arduino.h>
#include "ShiftIn.h"
#include "time_util.h"
#include <TimeAlarms.h>
#include <LiquidCrystal_I2C.h>

int buzzer = 12;
int PL = 2;
// CE pin 15
int CE = 18;
// Q7 pin 7
int DATA = 5;
// CP pin 2
int CLK_CP = 4;
int cursorRow = 0;
const int numOfRegisters = 2;
const int numBits = numOfRegisters * 8;
String day_of_the_week[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
String times_of_the_day[3] = {"Morning", "Afternoon", "Evening"};

ShiftIn<numOfRegisters> shift;
LiquidCrystal_I2C lcd(0x27, 16, 4); // I2C address 0x27, 16 column and 4 rows

void turnOnOrOffLCD(int time_of_day, int day, int turn_on)
{
  if (turn_on == -1)
  {
    lcd.clear();
    return;
  }
  else if (turn_on == 0)
  {
    lcd.clear();
    lcd.print("Pills successfully taken!");
    delay(2000);
    lcd.clear();
    lcd.print("Please close the compartment.");
    delay(2000);
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("------REMINDER------");
  lcd.setCursor(0, 1);
  lcd.print("Day: " + day_of_the_week[day]);
  lcd.setCursor(0, 2);
  lcd.print("Time: " + times_of_the_day[time_of_day]);
}

void activateReminder(int time_of_day, int day)
{
  turnOnOrOffLCD(time_of_day, day, true);
  tone(buzzer, 31);
  delay(2000);
  Serial.println(day * 3 + time_of_day);
  Serial.println("Day: " + String(day) + ", Time of Day: " + String(time_of_day));
  noTone(buzzer);
}

TimeUtil timeUtil(activateReminder);

std::vector<int> getCurrentStates()
{
  std::vector<int> states;

  for (int i = 0; i < 8 * numOfRegisters; i++)
  {

    int bitVal = shift.state(i) ? 1 : 0; // read single bit
    Serial.print(bitVal);
    if (bitVal == 1)
    {
      states.push_back(i);
    }
  }
  Serial.println();
  return states;
}

void setup()
{
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  shift.begin(PL, CE, DATA, CLK_CP);
  timeUtil.configureSetup();
  timeUtil.setDayAlarm(21, 33);
  timeUtil.setNoonAlarm(19, 15);
  timeUtil.setNightAlarm(23, 42);

  lcd.init();
  lcd.backlight();
}

void displayValues()
{
  for (int i = 0; i < shift.getDataWidth(); i++)
    Serial.print(shift.state(i));
  Serial.println();
}

void checkIfCorrectCompartmentOpened()
{
  std::vector<int> currentStates = getCurrentStates();
  std::vector<int> timeInfo = timeUtil.getCurrentTimeInfo();
  int currentDay = timeInfo[0];
  int currentTimeOfDay = timeInfo[1];

  int expectedCompartment = currentDay * 3 + currentTimeOfDay;

  if (currentStates[0] == -1)
    return;

  if (currentStates[0] == expectedCompartment)
  {
    turnOnOrOffLCD(currentTimeOfDay, currentDay, 0); // turn off with success message
  }
  else if (currentStates.size() > 1)
  {
    turnOnOrOffLCD(currentTimeOfDay, currentDay, -1);
    lcd.print("Please close all compartments ");
  }
  else
  {
    turnOnOrOffLCD(currentTimeOfDay, currentDay, -1);
    lcd.print("Wrong compartment opened!");
  }
}

void loop()
{
  if (shift.update())
  {
    Serial.println("Shift register updated:");
    getCurrentStates();
    checkIfCorrectCompartmentOpened();
  }
  timeUtil.refresh();
  delay(10);
}