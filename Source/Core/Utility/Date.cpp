/****************************************************************************/
/**
 *  @file Date.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Date.cpp 1129 2012-05-02 10:51:39Z s.yamada0808@gmail.com $
 */
/****************************************************************************/
#include "Date.h"
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <kvs/Tokenizer>

namespace
{

const char* MonthName[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

const char* MonthShortName[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

const char* DayOfWeekName[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

const char* DayOfWeekShortName[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

const size_t DaysInMonth[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

} // end of namespace


namespace kvs
{

Date::Date()
{
    this->today();
}

Date::Date( const long julian_day ):
    m_julian_day( julian_day )
{
    *this = this->convert_from_julian_date( m_julian_day );
}

Date::Date( const int year, const int month, const int day ):
    m_year( year ),
    m_month( month ),
    m_day( day )
{
    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );
}

Date::Date( const Date& date ):
    m_year( date.m_year ),
    m_month( date.m_month ),
    m_day( date.m_day )
{
    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );
}

Date::~Date()
{
}

Date& Date::operator = ( const Date& date )
{
    m_year = date.m_year;
    m_month = date.m_month;
    m_day = date.m_day;
    m_julian_day = date.m_julian_day;

    return *this;
}

Date& Date::operator += ( const int days )
{
    this->addDays( days );

    return *this;
}

Date& Date::operator -= ( const int days )
{
    this->subDays( days );
    return *this;
}

Date operator + ( const Date& date, const int days )
{
    Date ret( date ); ret += days;
    return ret;
}

Date operator - ( const Date& date, const int days )
{
    Date ret( date ); ret -= days;
    return ret;
}

int operator - ( const Date& date1, const Date& date2 )
{
    return date1.m_julian_day - date2.m_julian_day;
}

bool operator > ( const Date& date1, const Date& date2 )
{
    return date1.m_julian_day > date2.m_julian_day;
}

bool operator >= ( const Date& date1, const Date& date2 )
{
    return date1.m_julian_day >= date2.m_julian_day;
}

bool operator < ( const Date& date1, const Date& date2 )
{
    return date1.m_julian_day < date2.m_julian_day;
}

bool operator <= ( const Date& date1, const Date& date2 )
{
    return date1.m_julian_day <= date2.m_julian_day;
}

bool operator == ( const Date& date1, const Date& date2 )
{
    return date1.m_julian_day == date2.m_julian_day;
}

bool operator != ( const Date& date1, const Date& date2 )
{
    return date1.m_julian_day != date2.m_julian_day;
}

std::ostream& operator << ( std::ostream& os, const Date& date )
{
    os << date.dayOfWeekString() << " "
       << date.monthString()     << " "
       << date.day()             << " "
       << date.year();

    return os;
}

std::string Date::monthString( const bool abbreviation ) const
{
    return abbreviation ?
           std::string( ::MonthShortName[m_month-1] ) :
           std::string( ::MonthName[m_month-1] );
}

const Date& Date::today()
{
    time_t t = time( NULL );
    tm* time = localtime( &t );

    m_year = time->tm_year + 1900;
    m_month = time->tm_mon + 1;
    m_day = time->tm_mday;
    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );

    return *this;
}

int Date::yearsOld() const
{
    Date today;
    if ( today <= *this ) { return 0; }

    const long t = today.julianDay();
    const long b = this->julianDay();

    return static_cast<int>( ( t - b ) / 365.2425 );
}

Date::DayOfWeek Date::dayOfWeek() const
{
    struct tm t;
    t.tm_year = m_year - 1900;
    t.tm_mon = m_month - 1;
    t.tm_mday = m_day;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_isdst = -1;

    if ( mktime( &t ) == -1 ) { return Date::UnknownDay; }

    return ( Date::DayOfWeek )( t.tm_wday );
}

std::string Date::dayOfWeekString( const bool abbreviation ) const
{
    const Date::DayOfWeek day_of_week = this->dayOfWeek();
    if ( day_of_week == Date::UnknownDay ) { return "Unknown"; }

    return abbreviation ?
           std::string( ::DayOfWeekShortName[day_of_week] ) :
           std::string( ::DayOfWeekName[day_of_week] );
}

int Date::daysInMonth() const
{
    if ( ( m_month < Date::Jan ) || ( Date::Dec < m_month ) ) { return 0; }
    if ( ( Date::Feb == m_month ) && this->isLeepYear() ) { return 29; }

    return ::DaysInMonth[ m_month - 1 ];
}

std::string Date::toString( const std::string sep ) const
{
    char y[5]; sprintf( y, "%04d", m_year );
    char m[3]; sprintf( m, "%02d", m_month );
    char d[3]; sprintf( d, "%02d", m_day );

    return std::string( y ) + sep + std::string( m ) + sep + std::string( d );
}

void Date::fromString( const std::string date, const std::string sep )
{
    if ( sep.size() == 0 )
    {
        if ( date.size() == 8 )
        {
            // date: YYYYMMDD
            const std::string y = date.substr( 0,4 );
            const std::string m = date.substr( 4,2 );
            const std::string d = date.substr( 6,2 );

            m_year = atoi( y.c_str() );
            m_month = atoi( m.c_str() );
            m_day = atoi( d.c_str() );
        }
    }
    else
    {
        // date: YYYY?MM?DD ( '?': separator )
        kvs::Tokenizer tokenizer( date, sep );
        const std::string y = tokenizer.token();
        const std::string m = tokenizer.token();
        const std::string d = tokenizer.token();

        m_year = atoi( y.c_str() );
        m_month = atoi( m.c_str() );
        m_day = atoi( d.c_str() );
    }

    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );
}

bool Date::isLeepYear() const
{
    return !( m_year % 4 ) && ( m_year % 100 || !( m_year % 400 ) );
}

bool Date::isValid() const
{
    return m_year &&
           m_month >= Date::Jan && m_month <= Date::Dec &&
           m_day >= 1 && m_day <= this->daysInMonth();
}

void Date::addYears( const int years )
{
    m_year += years;
    this->adjust_days();

    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );
}

void Date::subYears( const int years )
{
    m_year -= years;
    this->adjust_days();

    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );
}

void Date::addMonths( const int months )
{
    m_month += months;
    if ( m_month > Date::Dec )
    {
        while ( m_month > Date::Dec )
        {
            m_month -= 12;
            m_year += 1;
        }
    }

    this->adjust_days();

    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );
}

void Date::subMonths( const int months )
{
    m_month -= months;

    if ( months > Date::Dec )
    {
        const int t = months / 12;
        m_year -= t;
        m_month += t * 12;
    }

    if ( m_month < Date::Jan )
    {
        m_year -=  1;
        m_month += 12;
    }

    this->adjust_days();

    m_julian_day = this->convert_to_julian_date( m_year, m_month, m_day );
}

void Date::addDays( const int days )
{
    m_julian_day += days;

    *this = this->convert_from_julian_date( m_julian_day );
}

void Date::subDays( const int days )
{
    m_julian_day -= days;

    *this = this->convert_from_julian_date( m_julian_day );
}

void Date::adjust_days()
{
    const Date this_month( m_year, m_month,     1 );
    const Date next_month( m_year, m_month + 1, 1 );

    const int ndays = next_month.daysInMonth() - this_month.daysInMonth();
    while ( m_day > ndays ) --m_day;
}

long Date::convert_to_julian_date( const int year, const int month, const int day ) const
{
    const int a = ( 14 - month ) / 12;
    const int m = month + 12 * a - 3;
    const int y = year + 4800 - a;

    return day + ( ( 153 * m + 2 ) / 5 ) + ( 365 * y ) + ( y / 4 ) - ( y / 100 ) + ( y / 400 ) - 32045;
}

Date Date::convert_from_julian_date( const long julian_day ) const
{
    const long a = julian_day + 32044;
    const long b = ( 4 * a + 3 ) / 146097;
    const long c = a - ( ( 146097 * b ) / 4 );
    const long d = ( 4 * c + 3 ) / 1461;
    const long e = c - ( 1461 * d / 4 );
    const long m = ( 5 * e + 2 ) / 153;

    const int day = e - ( ( 153 * m + 2 ) / 5 ) + 1;
    const int month = m + 3 - ( 12 * ( m / 10 ) );
    const int year = ( 100 * b ) + d - 4800 + ( m / 10 );

    return Date( year, month, day );
}

} // end of namespace kvs
