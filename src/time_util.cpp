#include <time.h>
#include <WiFi.h>
#include "sntp.h"
#include <CronAlarms.h>
#include <iostream>
#include <Arduino.h>
#include "time_util.h"

const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *time_zone = "IST-5:30";
int currentDay;

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

    Serial.println("\nTime synchronized!");
    Serial.println(&timeinfo, "Current time: %A, %B %d %Y %H:%M:%S");
}
void dayAlarmCallback()
{
    TimeUtil::alarmCallbackToChangeCircuit(0, currentDay);
}

void noonAlarmCallback()
{
    TimeUtil::alarmCallbackToChangeCircuit(1, currentDay);
}

void nightAlarmCallback()
{
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

void TimeUtil::refresh()
{
    Cron.delay();
    setCurrentDay();
    delay(10); // Wokwi delay to speed up the simulation
}