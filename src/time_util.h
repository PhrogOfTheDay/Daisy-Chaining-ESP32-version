#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#ifndef TimeUtil_H
#define TimeUtil_H
class TimeUtil
{
private:
    void (*alarmCallbackToChangeCircuit)(int, int);

public:
    TimeUtil(void (*func)(int, int));
    void connectToWiFi();
    void synchronizeTime();
    void setDayAlarm(int hour, int minute);
    void setNoonAlarm(int hour, int minute);
    void setNightAlarm(int hour, int minute);
    void configureSetup();
    void refresh();
};
#endif