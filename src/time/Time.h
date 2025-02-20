#ifndef DMTIME_H
#define DMTIME_H

#include "pico/stdlib.h"
#include "ds3231/SimpleDS3231.h"
#include "pico/util/datetime.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "hardware/rtc.h"
#include "network-helpers/NTP.h"
#include <math.h>
#include "debug/PrettySerial.h"
#include "network-helpers/ConnectionSettings.h"

#define DM_TIME_NAME "Time"

class Time{
    public:
        static void Init();
        static long GetUnixTimestamp();
        static bool GetTime(datetime_t* time);
        static uint32_t GetMsSinceBoot();
        static void SyncExternal();
        static bool SyncNTP(ConnectionSettings::NTPSettings *ntpServer);
        static void PrintTime();
        static uint8_t GetTemp();
    
    private:
        inline static SimpleDS3231 ds3231;
        static int CalcWeekday(int day, int month, int year);
};

#endif