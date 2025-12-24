#include <Neza74HC165.h>
#include <Adafruit_NeoPixel.h>
#include <vector>
#include <Arduino.h>
#include "ShiftIn.h"
#include "time_util.h"
#include <TimeAlarms.h>

int const LED_PIN = 15;
int const LED_COUNT = 21;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int PL = 18;
int CLK_CP = 17;
int CE = 19;
int DATA = 16;
const int numOfRegisters = 2;
const int numBits = numOfRegisters * 8;

Neza74HC165<numOfRegisters> shiftRegs;
ShiftIn<numOfRegisters> shift;

void turnOnOrOffLED(int led_no)
{
  strip.clear();
  strip.setPixelColor(led_no, 255, 0, 0);
  strip.show();
  delay(4000);
}

void activateReminder(int time_of_day, int day)
{
  turnOnOrOffLED(day * 3 + time_of_day);
  Serial.println("Reminder Activated!");
  Serial.println("Day: " + String(day) + ", Time of Day: " + String(time_of_day));
}

TimeUtil timeUtil(activateReminder);

std::vector<int> getCurrentStates()
{
  String day_of_the_week[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  String times_of_the_day[3] = {"Morning", "Afternoon", "Evening"};

  std::vector<int> states;

  shiftRegs.update();

  for (int i = 0; i < 8 * numOfRegisters; i++)
  {

    int bitVal = shift.state(i) ? 1 : 0; // read single bit
    if (bitVal == 1)
    {
      states.push_back(i);
      turnOnOrOffLED(i);
      Serial.println("Day: " + day_of_the_week[i / 3]);
      Serial.println("Time: " + times_of_the_day[i % 3]);
    }
  }
  Serial.println();

  return states;
}

void activateReminder2()
{
  Serial.println("Reminder Activated!");
}

void setup()
{
  Serial.begin(115200);
  shift.begin(PL, CE, DATA, CLK_CP);
  timeUtil.configureSetup();
  timeUtil.setDayAlarm(12, 51);
}

void displayValues()
{
  for (int i = 0; i < shift.getDataWidth(); i++)
    Serial.print(shift.state(i));
  Serial.println();
}

void loop()
{
  if (shift.update()) // read in all values. returns true if any button has changed
    getCurrentStates();
  timeUtil.refresh();
  delay(10);
}