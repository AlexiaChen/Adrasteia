#include <iostream>
#include <ctime>
#include <cassert>
#include <boost/date_time/gregorian/gregorian.hpp>
// 格力高利历 1400-01-01 to 9999-12-31
int main()
{
    boost::gregorian::date d1(2010,1,1);
    boost::gregorian::date d2(2010,1,2);
    boost::gregorian::date d3(d1);

    assert(d1 < d2);
    assert(d1 == d3);

    boost::gregorian::date d4 = boost::gregorian::from_string("2015-12-1");

    std::cout << "d4's year: " << d4.year_month_day().year << std::endl;
    std::cout << "d4's month: " << d4.year_month_day().month << std::endl;
    std::cout << "d4's day: " << d4.year_month_day().day << std::endl;
    std::cout << "d4's day of week: " << d4.day_of_week() << std::endl; // 星期几
    std::cout << "d4's day of year: " << d4.day_of_year() << std::endl; // 一年中的第几天
    std::cout << "d4's week number: " << d4.week_number() << std::endl; // 一年中的第几周

    std::cout << "simple string: "<< boost::gregorian::to_simple_string(d4) << std::endl;
    std::cout << "iso string: "<< boost::gregorian::to_iso_string(d4) << std::endl;
    std::cout << "iso extend string: "<< boost::gregorian::to_iso_extended_string(d4) << std::endl;
    std::cout << "sql string: " <<    boost::gregorian::to_sql_string(d4) << std::endl;
    std::cout << "date  : " << d4 << std::endl;

    tm t = boost::gregorian::to_tm(d4);

    std::cout << "tm year: " << t.tm_year << std::endl;
    std::cout << "tm month: " << t.tm_mon << std::endl;
    std::cout << "tm month day: " << t.tm_mday << std::endl;

    boost::gregorian::date d5(boost::gregorian::date_from_tm(t));
    assert(d5 == d4);

    boost::gregorian::date currentDate = boost::gregorian::day_clock::local_day(); // local date
    boost::gregorian::date utcDate = boost::gregorian::day_clock::universal_day(); // utc date

    std::cout << "local date: " << currentDate << std::endl;
    std::cout << "utc date: " << utcDate << std::endl;

    boost::gregorian::days  dd1(10), dd2(-100), dd3(225);     // 一个时间长度，以天为计数
    boost::gregorian::weeks w(3);                             // 时间长度，以周为计数
    boost::gregorian::months mon(5);                          // 时间长度，以月为计数
    boost::gregorian::years year(2);                          // 时间长度，以年为计数

    assert(dd1 > dd2 && dd1 < dd3);
    assert(dd1 + dd2 == boost::gregorian::days(-90));
    assert((dd1 + dd3).days() == 235);
    assert(dd1 / 2 == boost::gregorian::days(5));
    
    assert(w.days() == 21);

    boost::gregorian::months m2 = year + mon;    // 2年零5个月

    assert(m2.number_of_months() == 29);
    assert((year * 2).number_of_years() == 4); 

    return 0;
}