#define BLYNK_TEMPLATE_ID "TMPL6wztc6zoe"
#define BLYNK_TEMPLATE_NAME "My Template "
#define BLYNK_AUTH_TOKEN "4yNUa-WC4_9O9m41CLEWgttozOh2Srp7"

#include <Neza74HC165.h>
#include <Adafruit_NeoPixel.h>
#include <vector>
#include <Arduino.h>
#include "ShiftIn.h"
#include "time_util.h"
#include <TimeAlarms.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <Preferences.h>

// credentials for Blynk
char network_ssid[] = "Leo_EXT";
char pass[] = "Asitha123@@@@";

/*char network_ssid[] = "Wokwi-GUEST";
char pass[] = "";*/
// eolc6468
char auth[] = "4yNUa-WC4_9O9m41CLEWgttozOh2Srp7";

int buzzer = 13;
int PL = 2;
int CE = 18;
int DATA = 5;
int CLK_CP = 4;
int cursorRow = 0;

const int numOfRegisters = 2;
const int numBits = numOfRegisters * 8;
String day_of_the_week[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
String times_of_the_day[3] = {"Morning", "Afternoon", "Evening"};

ShiftIn<numOfRegisters> shift;
LiquidCrystal_I2C lcd(0x27, 16, 4); // I2C address 0x27, 16 column and 4 rows
Preferences prefs;

bool timeToTakeMedicine = false;

void turnOnOrOffLCD(int time_of_day, int day, int turn_on)
{
  if (turn_on == -1)
  {
    lcd.clear();
    return;
  }
  else if (turn_on == 0)
  {
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.print("Pills successfully taken!");
    delay(2000);
    lcd.clear();
    lcd.print("Please close the compartment.");
    delay(2000);
    Blynk.logEvent("pills_taken", "User successfully took their " + times_of_the_day[time_of_day] + " medicine for " + day_of_the_week[day] + ".");

    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("----REMINDER----");
  lcd.setCursor(0, 1);
  lcd.print("Day: " + day_of_the_week[day]);
  lcd.setCursor(0, 2);
  lcd.print("Time: " + times_of_the_day[time_of_day]);
}

void activateReminder(int time_of_day, int day)
{
  timeToTakeMedicine = true;
  digitalWrite(buzzer, HIGH);
  Serial.println("Day: " + String(day) + ", Time of Day: " + String(time_of_day));
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
      Serial.println("Day and Time of Day for opened compartment: " + String(i / 3) + ", " + String(i % 3));
    }
  }
  Serial.println();
  return states;
}

int readFromMemory(String variable_name)
{
  prefs.begin("timeData", true);
  int value = prefs.getInt(variable_name.c_str(), -1);
  prefs.end();
  return value;
}

void initializeMemory()
{
  prefs.begin("timeData", true);
  if (readFromMemory("morningTimeHr") == -1)
  {
    prefs.putInt("morningTimeHr", 9);
  }
  if (readFromMemory("morningTimeMin") == -1)
  {
    prefs.putInt("morningTimeMin", 0);
  }
  if (readFromMemory("noonTimeHr") == -1)
  {
    prefs.putInt("noonTimeHr", 13);
  }
  if (readFromMemory("noonTimeMin") == -1)
  {
    prefs.putInt("noonTimeMin", 0);
  }
  if (readFromMemory("eveningTimeHr") == -1)
  {
    prefs.putInt("eveningTimeHr", 21);
  }
  if (readFromMemory("eveningTimeMin") == -1)
  {
    prefs.putInt("eveningTimeMin", 0);
  }
}

void writeToMemory(String variable_name, int value)
{
  prefs.begin("timeData", false);
  prefs.putInt(variable_name.c_str(), value);
  prefs.end();
}

BLYNK_WRITE(V0) // read the morning time input
{
  String date = param.asString();
  TimeInputParam time(param);

  int h = time.getStartHour();
  int m = time.getStartMinute();

  timeUtil.setDayAlarm(h, m);
  writeToMemory("morningTimeHr", h);
  writeToMemory("morningTimeMin", m);
  Serial.println("Morning time set to " + String(h) + ":" + String(m));
}

BLYNK_WRITE(V1) // read the afternoon time input
{
  String date = param.asString();
  TimeInputParam time(param);

  int h = time.getStartHour();
  int m = time.getStartMinute();
  timeUtil.setNoonAlarm(h, m);
  writeToMemory("noonTimeHr", h);
  writeToMemory("noonTimeMin", m);
  Serial.println("Afternoon time set to " + String(h) + ":" + String(m));
  Serial.println("Afternoon time HR from memory: " + String(readFromMemory("noonTimeHr")));
}

BLYNK_WRITE(V2) // read the evening time input
{
  String date = param.asString();
  TimeInputParam time(param);

  int h = time.getStartHour();
  int m = time.getStartMinute();

  timeUtil.setNightAlarm(h, m);

  writeToMemory("eveningTimeHr", h);
  writeToMemory("eveningTimeMin", m);
  Serial.println("Evening time set to " + String(h) + ":" + String(m));
  Serial.println("Evening time HR from memory: " + String(readFromMemory("eveningTimeHr")));
}

void setup()
{

  pinMode(PL, OUTPUT);
  pinMode(CE, OUTPUT);
  pinMode(CLK_CP, OUTPUT);
  pinMode(DATA, INPUT);

  Serial.begin(9600);
  Serial.println("Starting up...");
  Blynk.begin(auth, network_ssid, pass, "blynk.cloud", 80);
  Serial.println("Blynk connected.");

  pinMode(buzzer, OUTPUT);
  shift.begin(PL, CE, DATA, CLK_CP);
  timeUtil.configureSetup(); // just synchronize everything
  Serial.println("Time synchronized.");
  initializeMemory();

  timeUtil.setDayAlarm(readFromMemory("morningTimeHr"), readFromMemory("morningTimeMin"));
  timeUtil.setNoonAlarm(readFromMemory("noonTimeHr"), readFromMemory("noonTimeMin"));
  timeUtil.setNightAlarm(readFromMemory("eveningTimeHr"), readFromMemory("eveningTimeMin"));
  Serial.println("Memory initialized.");
  lcd.init();
  lcd.backlight();

  Blynk.logEvent("pills_taken", "The smart pill dispenser has been started.");
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
  int counter = 0;
  for (int x : currentStates)
  {
    counter += x;
  }
  Serial.println("Counter value: " + String(counter));
  if (counter == 0)
  {
    Serial.println("No compartments opened.");
    digitalWrite(buzzer, LOW);
    return;
  }

  if (!timeToTakeMedicine)
  {
    digitalWrite(buzzer, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kindly close the ");
    lcd.setCursor(0, 1);
    lcd.print("compartments");
    lcd.setCursor(0, 2);
    lcd.print("Its not time");
    lcd.setCursor(0, 3);
    lcd.print("to take medicine");
    delay(2000);
    lcd.clear();
    return;
  }

  int currentDay = timeInfo[0];
  int currentTimeOfDay = timeInfo[1];

  int expectedCompartment = currentDay * 3 + currentTimeOfDay;
  Serial.println("Expected compartment to be opened: " + String(expectedCompartment));

  if (currentStates.empty())
  {
    return;
  }

  if (currentStates[0] == -1)
  {
    return;
  }

  if (currentStates[0] == expectedCompartment)
  {
    turnOnOrOffLCD(currentTimeOfDay, currentDay, 0); // turn off with success message like "Success!"
    timeToTakeMedicine = false;
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
  Blynk.run();
  if (shift.update())
  {
    getCurrentStates();
    checkIfCorrectCompartmentOpened();
  }
  timeUtil.refresh();
  delay(100);
  for (int i = 0; i < 8 * numOfRegisters; i++)
  {

    int bitVal = shift.state(i) ? 1 : 0; // read single bit
    // read 0 when pressed down (default state), reads 1 when opened
    Serial.print(bitVal);
  }
  Serial.println();

  delay(1000);
}