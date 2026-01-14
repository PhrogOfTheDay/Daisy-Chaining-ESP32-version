#include <time.h>
#include <WiFi.h>
#include "sntp.h"
#include <CronAlarms.h>
#include <iostream>
#include <Arduino.h>
#include <vector>
#include "time_util.h"

const char *ssid = "Ravindu's Galaxy S10+";
const char *password = "eolc6468";
const char *time_zone = "IST-5:30";
int currentDay;
int currentTimeOfDay;

void (*TimeUtil::alarmCallbackToChangeCircuit)(int, int) = nullptr;

TimeUtil::TimeUtil(void (*func)(int, int))
{
    alarmCallbackToChangeCircuit = func;
}

void TimeUtil::connectToWiFi()
{
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500); // keep looping until successful connection established
    }
    Serial.println("\nConnected to the WiFi network");
}

void setCurrentDay()
{
    struct tm timeInfo;
    // getLocalTime returns true if all data/time info was successfully loaded into timeInfo
    if (getLocalTime(&timeInfo))
    {
        int todaysDay = timeInfo.tm_wday == 0 ? 6 : timeInfo.tm_wday - 1;
        currentDay = todaysDay; // gets day of the week between 0-6 for each since sunday
    }
}

void TimeUtil::synchronizeTime()
{
    configTzTime(time_zone, "pool.ntp.org", "time.nist.gov");

    struct tm timeinfo;
    Serial.println("Waiting for NTP time sync...");

    while (!getLocalTime(&timeinfo))
    {
        Serial.print(".");
        delay(1000); // keep looping until successful sync
    }
    setCurrentDay();
    Serial.println("\nTime synchronized!");
    Serial.println(&timeinfo, "Current time: %A, %B %d %Y %H:%M:%S");
}

void dayAlarmCallback()
{
    setCurrentDay();
    currentTimeOfDay = 0;
    TimeUtil::alarmCallbackToChangeCircuit(0, currentDay);
}

void noonAlarmCallback()
{
    setCurrentDay();
    currentTimeOfDay = 1;
    TimeUtil::alarmCallbackToChangeCircuit(1, currentDay);
}

void nightAlarmCallback()
{
    setCurrentDay();
    currentTimeOfDay = 2;
    TimeUtil::alarmCallbackToChangeCircuit(2, currentDay);
}

void TimeUtil::setDayAlarm(int hour, int minute)
{
    // format: "second minute hour day month day-of-week"
    // example for daily at HH:MM:00 : "0 23 11 * * *"
    char buf[32];
    snprintf(buf, sizeof(buf), "0 %d %d * * *", minute, hour);

    Serial.print("Setting Alarm with Cron: ");
    Serial.println(buf);

    Cron.create(buf, dayAlarmCallback, false);
}

void TimeUtil::setNoonAlarm(int hour, int minute)
{
    // format: "second minute hour day month day-of-week"
    // example for daily at HH:MM:00 : "0 23 11 * * *"
    char buf[32];
    snprintf(buf, sizeof(buf), "0 %d %d * * *", minute, hour);

    Serial.print("Setting Alarm with Cron: ");
    Serial.println(buf);

    Cron.create(buf, noonAlarmCallback, false);
}

void TimeUtil::setNightAlarm(int hour, int minute)
{
    // format is "second minute hour day month day-of-week"
    // example for daily at HH:MM:00 : "0 23 11 * * *"
    char buf[32];
    snprintf(buf, sizeof(buf), "0 %d %d * * *", minute, hour);
    // sizeof(buf) used to prevent overflow, though it's not likely here

    Serial.print("Setting Alarm with Cron: ");
    Serial.println(buf);

    Cron.create(buf, nightAlarmCallback, false);
}

void TimeUtil::configureSetup()
{
    connectToWiFi();
    synchronizeTime();
}

std::vector<int> TimeUtil::getCurrentTimeInfo()
{
    struct tm *timeInfo;
    Serial.println("Fetching current time info...");

    Serial.println("Current time" + currentTimeOfDay);
    Serial.println("Current day" + currentDay);
    return {currentDay, currentTimeOfDay};
}

void TimeUtil::refresh()
{
    Cron.delay();
    setCurrentDay();
    delay(10); // Wokwi delay to speed up the simulation
}