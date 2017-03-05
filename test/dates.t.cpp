////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2017, Göteborg Bit Factory.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <test.h>
#include <Datetime.h>

////////////////////////////////////////////////////////////////////////////////
void testInit (UnitTest& t, const std::string& value, Datetime& var)
{
  try
  {
    var = Datetime (value);
    t.pass (value + " --> valid");
  }

  catch (const std::string& e)
  {
    t.fail (value + " --> valid");
    t.diag (e);
  }

  catch (...)
  {
    t.fail (value + " --> valid");
  }
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (101);

  Datetime sunday;    testInit (t, "sunday",    sunday);
  Datetime monday;    testInit (t, "monday",    monday);
  Datetime tuesday;   testInit (t, "tuesday",   tuesday);
  Datetime wednesday; testInit (t, "wednesday", wednesday);
  Datetime thursday;  testInit (t, "thursday",  thursday);
  Datetime friday;    testInit (t, "friday",    friday);
  Datetime saturday;  testInit (t, "saturday",  saturday);

  Datetime sun; testInit (t, "sun", sun);
  Datetime mon; testInit (t, "mon", mon);
  Datetime tue; testInit (t, "tue", tue);
  Datetime wed; testInit (t, "wed", wed);
  Datetime thu; testInit (t, "thu", thu);
  Datetime fri; testInit (t, "fri", fri);
  Datetime sat; testInit (t, "sat", sat);

  t.ok (sunday    == sun, "sunday == sun");
  t.ok (monday    == mon, "monday == mon");
  t.ok (tuesday   == tue, "tuesday == tue");
  t.ok (wednesday == wed, "wednesday == wed");
  t.ok (thursday  == thu, "thursday == thu");
  t.ok (friday    == fri, "friday == fri");
  t.ok (saturday  == sat, "saturday == sat");

  Datetime january;   testInit (t, "january",   january);
  Datetime february;  testInit (t, "february",  february);
  Datetime march;     testInit (t, "march",     march);
  Datetime april;     testInit (t, "april",     april);
  Datetime may;       testInit (t, "may",       may);
  Datetime june;      testInit (t, "june",      june);
  Datetime july;      testInit (t, "july",      july);
  Datetime august;    testInit (t, "august",    august);
  Datetime september; testInit (t, "september", september);
  Datetime october;   testInit (t, "october",   october);
  Datetime november;  testInit (t, "november",  november);
  Datetime december;  testInit (t, "december",  december);

  Datetime jan; testInit (t, "jan", jan);
  Datetime feb; testInit (t, "feb", feb);
  Datetime mar; testInit (t, "mar", mar);
  Datetime apr; testInit (t, "apr", apr);
  Datetime jun; testInit (t, "jun", jun);
  Datetime jul; testInit (t, "jul", jul);
  Datetime aug; testInit (t, "aug", aug);
  Datetime sep; testInit (t, "sep", sep);
  Datetime oct; testInit (t, "oct", oct);
  Datetime nov; testInit (t, "nov", nov);
  Datetime dec; testInit (t, "dec", dec);

  t.ok (january   == jan, "january == jan");
  t.ok (february  == feb, "february == feb");
  t.ok (march     == mar, "march == mar");
  t.ok (april     == apr, "april == apr");
  // May has only three letters.
  t.ok (june      == jun, "june == jun");
  t.ok (july      == jul, "july == jul");
  t.ok (august    == aug, "august == aug");
  t.ok (september == sep, "september == sep");
  t.ok (october   == oct, "october == oct");
  t.ok (november  == nov, "november == nov");
  t.ok (december  == dec, "december == dec");

  // Simply instantiate these for now.  Test later.
  Datetime now;            testInit (t, "now", now);
  Datetime today;          testInit (t, "today", today);
  Datetime sod;            testInit (t, "sod", sod);
  Datetime yesterday;      testInit (t, "yesterday", yesterday);
  Datetime tomorrow;       testInit (t, "tomorrow", tomorrow);
  Datetime eod;            testInit (t, "eod", eod);
  Datetime soy;            testInit (t, "soy", soy);
  Datetime eoy;            testInit (t, "eoy", eoy);
  Datetime socm;           testInit (t, "socm", socm);
  Datetime eocm;           testInit (t, "eocm", eocm);
  Datetime soww;           testInit (t, "soww", soww);
  Datetime eoww;           testInit (t, "eoww", eoww);
  Datetime som;            testInit (t, "som", som);
  Datetime eom;            testInit (t, "eom", eom);
  Datetime later;          testInit (t, "later", later);
  Datetime someday;        testInit (t, "someday", someday);
  Datetime easter;         testInit (t, "easter", easter);
  Datetime eastermonday;   testInit (t, "eastermonday", eastermonday);
  Datetime ascension;      testInit (t, "ascension", ascension);
  Datetime pentecost;      testInit (t, "pentecost", pentecost);
  Datetime goodfriday;     testInit (t, "goodfriday", goodfriday);
  Datetime midsommar;      testInit (t, "midsommar", midsommar);
  Datetime midsommarafton; testInit (t, "midsommarafton", midsommarafton);
  Datetime juhannus;       testInit (t, "juhannus", juhannus);
  Datetime first;          testInit (t, "1st", first);
  Datetime second;         testInit (t, "2nd", second);
  Datetime third;          testInit (t, "3rd", third);
  Datetime fourth;         testInit (t, "4th", fourth);

  // Check abbreviations.
  // TW-1515: abbreviation.minimum does not apply to date recognition
  Datetime yesterday2;      testInit (t, "yesterday", yesterday2);
  Datetime yesterday3;      testInit (t, "yesterda",  yesterday3);
  Datetime yesterday4;      testInit (t, "yesterd",   yesterday4);
  Datetime yesterday5;      testInit (t, "yester",    yesterday5);
  Datetime yesterday6;      testInit (t, "yeste",     yesterday6);
  Datetime yesterday7;      testInit (t, "yest",      yesterday7);
  Datetime yesterday8;      testInit (t, "yes",       yesterday8);

  t.ok (now >= today,               "now >= today");
  t.ok (sod == tomorrow,            "sod == tomorrow");
  t.ok (yesterday < today,          "yesterday < today");
  t.ok (today < tomorrow,           "today < tomorrow");
  t.ok (socm < eocm,                "socm < eocm");
  t.ok (now < later,                "now < later");
  t.ok (now < someday,              "now < someday");
  t.ok (goodfriday < easter,        "goodfriday < easter");
  t.ok (easter < eastermonday,      "easter < eastermonday");
  t.ok (midsommarafton < midsommar, "midsommarafton < midsommar");
  t.ok (juhannus == midsommarafton, "juhannus == midsommarafton");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
