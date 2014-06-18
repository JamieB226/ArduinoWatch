//
//  Time.cpp
//  
//
//  Created by Jamie Booth on 2/14/14.
//
//

#include "Time.h"

void Time::setTime(uint8_t hr, uint8_t min, uint8_t sec, uint16_t ms)
{
    if ((hr >= 0) && (hr<24))
    {
        hours=hr;
    }
    if ((min >= 0) && (min<60))
    {
        minutes=min;
    }
    if ((sec >= 0) && (sec<60))
    {
        seconds=sec;
    }
    if ((ms >= 0) && (ms<1000))
    {
        milliseconds=ms;
    }
}

uint8_t Time::getHours()
{
    return hours;
}

uint8_t Time::getAMPMHours()
{
    uint8_t tempHours;
    
    if(hours >12)
    {
        tempHours=hours-12;
    }
    else
    {
        tempHours=hours;
    }
    
    if(hours == 0)
    {
        tempHours=12;
    }
    
    return tempHours;
}

uint8_t Time::getMinutes()
{
    return minutes;
}

uint8_t Time::getSeconds()
{
    return seconds;
}

uint16_t Time::getMilliseconds()
{
    return milliseconds;
}

String Time::toString()
{
    String retval;
    
    retval+=hours;
    retval+=":";
    retval+=minutes;
    retval+=":";
    retval+=seconds;
    retval+=".";
    retval+=milliseconds;
    
    return retval;
}

void Time::addHours(uint8_t hr)
{
    if ((hr+hours) < 24)
    {
        hours+=hr;
    }
    else
    {
        hours+=hr;
        hours-=24;
    }
}

void Time::addMinutes(uint8_t min)
{
    if ((min+minutes) < 60)
    {
        minutes+=min;
    }
    else
    {
        minutes+=min;
        minutes-=60;
        addHours(1);
    }
}

void Time::addSeconds(uint8_t sec)
{
    if ((sec+seconds) < 60)
    {
        seconds+=sec;
    }
    else
    {
        seconds+=sec;
        seconds-=60;
        addMinutes(1);
    }
}

void Time::addMilliseconds(uint16_t ms)
{
    if ((ms+milliseconds) < 1000)
    {
        milliseconds+=ms;
    }
    else
    {
        milliseconds+=ms;
        milliseconds-=1000;
        addSeconds(1);
    }
}

Time Time::compareTime(Time otherTime)
{
    Time retTime;
    
    if (hours < otherTime.getHours()) {
        retTime.addHours(24+hours-otherTime.getHours());
    }
    else
    {
        retTime.addHours(hours-otherTime.getHours());
    }
    if (minutes < otherTime.getMinutes())
    {
        retTime.addMinutes(60+minutes-otherTime.getMinutes());
    }
    else
    {
        retTime.addMinutes(minutes-otherTime.getMinutes());
    }
    
    if (seconds < otherTime.getSeconds())
    {
        retTime.addSeconds(60+seconds-otherTime.getSeconds());
    }
    else
    {
        retTime.addSeconds(seconds-otherTime.getSeconds());
    }
    
    if (milliseconds < otherTime.getMilliseconds())
    {
        retTime.addMilliseconds(60+milliseconds-otherTime.getMilliseconds());
    }
    else
    {
        retTime.addMilliseconds(milliseconds-otherTime.getMilliseconds());
    }
    
    return retTime;
}

Time::Time()
{
    hours=0;
    minutes=0;
    seconds=0;
    milliseconds=0;
}

Date::Date()
{
    year=2014;
    month=1;
    day=1;
}

String Date::toString()
{
    String retval="";
    
    retval.concat(year);
    retval.concat("/");
    retval.concat(month);
    retval.concat("/");
    retval.concat(day);
    
    return  retval;
}

uint16_t Date::getYear()
{
    return year;
}

uint8_t Date::getMonth()
{
    return month;
}

uint8_t Date::getDay()
{
    return day;
}

String Date::getMonthName()
{
    String retval;
    
    switch (month) {
        case 1:
            retval="January";
            break;
        case 2:
            retval="February";
            break;
        case 3:
            retval="March";
            break;
        case 4:
            retval="April";
            break;
        case 5:
            retval="May";
            break;
        case 6:
            retval="June";
            break;
        case 7:
            retval="July";
            break;
        case 8:
            retval="August";
            break;
        case 9:
            retval="September";
            break;
        case 10:
            retval="October";
            break;
        case 11:
            retval="November";
            break;
        case 12:
            retval="December";
            break;
            
        default:
            retval="";
            break;
    }
    return retval;
}

void Date::setDate(uint16_t newyear,uint8_t newmonth,uint8_t newday)
{
    if (newyear > 0)
    {
        year=newyear;
    }
    if (newmonth>=1 && newmonth<=12)
    {
        month=newmonth;
    }
    if (newday>=1 && newday<=31)
    {
        day=newday;
    }
}

void Date::addDay()
{
    day++;
    if (day > daysPerMonth(month,year))
    {
        day=day-daysPerMonth(month,year);
        month++;
        if (month>12)
        {
            month=1;
            year++;
        }
    }
}

bool Date::isLeapYear(uint16_t testYear)
{
    if ( (((testYear % 4) == 0) && ((testYear % 100) != 0)) || ((testYear % 400) == 0) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t Date::daysPerMonth(uint8_t testMonth,uint16_t testYear)
{
    uint8_t retval;
    
    switch (testMonth)
    {
        case 1:
            retval=31;
            break;
        case 2:
            if (isLeapYear(testYear))
            {
                retval=29;
            }
            else
            {
                retval=28;
            }
            break;
        case 3:
            retval=31;
            break;
        case 4:
            retval=30;
            break;
        case 5:
            retval=31;
            break;
        case 6:
            retval=30;
            break;
        case 7:
            retval=31;
            break;
        case 8:
            retval=31;
            break;
        case 9:
            retval=30;
            break;
        case 10:
            retval=31;
            break;
        case 11:
            retval=30;
            break;
        case 12:
            retval=31;
            break;
            
        default:
            retval=30;
            break;
    }
    return retval;
}

uint8_t Date::getDoW()
{
    static uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    uint8_t m,d;
    uint16_t y;
    
    y=year;
    m=month;
    d=day;
    
    y -= m < 3;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}