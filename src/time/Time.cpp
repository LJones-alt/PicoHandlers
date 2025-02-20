#include "Time.h"

//Initialise time!
void Time::Init()
{
    sleep_ms(1);
    rtc_init();
    SyncExternal();    
    sleep_us(64);    
}

//Gets unixtimestamp time from internal clock
long Time::GetUnixTimestamp()
{
    datetime_t time;
    GetTime(&time);

    struct tm t;    
    t.tm_year = (int)time.year -1900;  // Year - 1900
    t.tm_mon = (int)time.month-1;         // Month, where 0 = jan
    t.tm_mday = (int)time.day;     // Day of the month
    t.tm_hour = (int)time.hour;
    t.tm_min = (int)time.min;
    t.tm_sec = (int)time.sec;    
    t.tm_isdst = 0;        // Is DST on? 1 = yes, 0 = no, -1 = unknown

    time_t t_of_day;    
    t_of_day = mktime(&t);
   //long unixseconds = (long)t_of_day;
   return (long)t_of_day;
}

int Time::CalcWeekday(int day, int month, int year)
{
    // from https://www.tutorialspoint.com/day-of-the-week-in-cplusplus
   int mon;
   if(month > 2)
      mon = month; //for march to december month code is same as month
   else{
      mon = (12+month); //for Jan and Feb, month code will be 13 and 14
      year--; //decrease year for month Jan and Feb
   }
   int y = year % 100; //last two digit
   int c = year / 100; //first two digit
   int w = (day + floor((13*(mon+1))/5) + y + floor(y/4) + floor(c/4) + (5*c));
   w = w % 7;
   return w;
}

//Gets DateTime from internal clock
bool Time::GetTime(datetime_t *time)
{    
    return rtc_get_datetime(time);
}

//Gets Millisecond count since boot from internal clock
uint32_t Time::GetMsSinceBoot()
{    
    return to_ms_since_boot(get_absolute_time());
}

//Syncs external and interal clock
void Time::SyncExternal()
{   
    PRINT_INFO(DM_TIME_NAME,"Synchronising internal clock with RTC");
    datetime_t t;
    t.year  = ds3231.get_year();
    t.month = ds3231.get_mon();
    t.day   = ds3231.get_day();
    t.dotw = CalcWeekday(t.year,t.month,t.day);
    t.hour  = ds3231.get_hou();
    t.min   = ds3231.get_min();
    t.sec   = ds3231.get_sec();
    rtc_set_datetime(&t);
}

//Syncs syncs clocks with NTP
bool Time::SyncNTP(ConnectionSettings::NTPSettings *ntpServer)
{
    PRINT_INFO(DM_TIME_NAME,"Synchronise clock with NTP");
    bool success = false;
    NTP* ntp = new NTP(ntpServer);
    tm* ntpTimeUTC = ntp->QueryBlocking();
    if(ntpTimeUTC != NULL)
    {
        ds3231.set_year(ntpTimeUTC->tm_year + 1900);
        ds3231.set_mon(ntpTimeUTC->tm_mon+1);
        ds3231.set_day(ntpTimeUTC->tm_mday);
        ds3231.set_hou(ntpTimeUTC->tm_hour,false,false);
        ds3231.set_min(ntpTimeUTC->tm_min);
        ds3231.set_sec(ntpTimeUTC->tm_sec);
        sleep_ms(50);
        success = true;
        PRINT_SUCCESS(DM_TIME_NAME,"Writing new time to RTC clock");
    }
    delete ntp;
    SyncExternal();
    return success;    
}

uint8_t Time::GetTemp()
{
    return ds3231.get_temp();
}

void Time::PrintTime()
{
    PRINT_INFO(DM_TIME_NAME,"Time on DS3231: %s -- %s -- %dC", ds3231.get_date_str(), ds3231.get_time_str(), ds3231.get_temp(), 248);
}