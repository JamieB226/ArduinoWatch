//
//  Time.h
//  
//
//  Created by Jamie Booth on 2/14/14.
//
//
#include <WString.h>

#ifndef _Time_h
#define _Time_h
#endif

class Time
{
public:
    void setTime(uint8_t hours,uint8_t minutes,uint8_t seconds, uint16_t milliseconds);
    
    uint8_t getHours();
    uint8_t getAMPMHours();
    uint8_t getMinutes();
    uint8_t getSeconds();
    uint16_t getMilliseconds();
    
    String toString();
    
    void addHours(uint8_t hr);
    void addMinutes(uint8_t min);
    void addSeconds(uint8_t sec);
    void addMilliseconds(uint16_t ms);
    
    Time compareTime(Time otherTime);
    
    Time ();
    
private:
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t milliseconds;
};

class Date
{
public:
    void setDate(uint16_t newyear,uint8_t newmonth, uint8_t newday);
    void addDay();
    static bool isLeapYear(uint16_t testYear);
    static uint8_t daysPerMonth(uint8_t testMonth,uint16_t testYear);
    String getMonthName();
    
    uint16_t getYear();
    uint8_t getMonth();
    uint8_t getDay();
    uint8_t getDoW();
    String toString();
    
    Date ();
    
private:
    uint16_t year;
    uint8_t month;
    uint8_t day;
};