////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Göteborg Bit Factory.
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
#include <Dates.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
void testInit (UnitTest& t, const std::string& value, Variant& var)
{
  try
  {
    namedDates (value, var);
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
  UnitTest t (104);

  Variant sunday;    testInit (t, "sunday",    sunday);
  Variant monday;    testInit (t, "monday",    monday);
  Variant tuesday;   testInit (t, "tuesday",   tuesday);
  Variant wednesday; testInit (t, "wednesday", wednesday);
  Variant thursday;  testInit (t, "thursday",  thursday);
  Variant friday;    testInit (t, "friday",    friday);
  Variant saturday;  testInit (t, "saturday",  saturday);

  Variant sun; testInit (t, "sun", sun);
  Variant mon; testInit (t, "mon", mon);
  Variant tue; testInit (t, "tue", tue);
  Variant wed; testInit (t, "wed", wed);
  Variant thu; testInit (t, "thu", thu);
  Variant fri; testInit (t, "fri", fri);
  Variant sat; testInit (t, "sat", sat);

  t.ok (sunday    == sun, "sunday == sun");
  t.ok (monday    == mon, "monday == mon");
  t.ok (tuesday   == tue, "tuesday == tue");
  t.ok (wednesday == wed, "wednesday == wed");
  t.ok (thursday  == thu, "thursday == thu");
  t.ok (friday    == fri, "friday == fri");
  t.ok (saturday  == sat, "saturday == sat");

  Variant january;   testInit (t, "january",   january);
  Variant february;  testInit (t, "february",  february);
  Variant march;     testInit (t, "march",     march);
  Variant april;     testInit (t, "april",     april);
  Variant may;       testInit (t, "may",       may);
  Variant june;      testInit (t, "june",      june);
  Variant july;      testInit (t, "july",      july);
  Variant august;    testInit (t, "august",    august);
  Variant september; testInit (t, "september", september);
  Variant october;   testInit (t, "october",   october);
  Variant november;  testInit (t, "november",  november);
  Variant december;  testInit (t, "december",  december);

  Variant jan; testInit (t, "jan", jan);
  Variant feb; testInit (t, "feb", feb);
  Variant mar; testInit (t, "mar", mar);
  Variant apr; testInit (t, "apr", apr);
  Variant jun; testInit (t, "jun", jun);
  Variant jul; testInit (t, "jul", jul);
  Variant aug; testInit (t, "aug", aug);
  Variant sep; testInit (t, "sep", sep);
  Variant oct; testInit (t, "oct", oct);
  Variant nov; testInit (t, "nov", nov);
  Variant dec; testInit (t, "dec", dec);

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
  Variant now;            testInit (t, "now", now);
  Variant today;          testInit (t, "today", today);
  Variant sod;            testInit (t, "sod", sod);
  Variant yesterday;      testInit (t, "yesterday", yesterday);
  Variant tomorrow;       testInit (t, "tomorrow", tomorrow);
  Variant eod;            testInit (t, "eod", eod);
  Variant soy;            testInit (t, "soy", soy);
  Variant eoy;            testInit (t, "eoy", eoy);
  Variant socm;           testInit (t, "socm", socm);
  Variant eocm;           testInit (t, "eocm", eocm);
  Variant soww;           testInit (t, "soww", soww);
  Variant eoww;           testInit (t, "eoww", eoww);
  Variant som;            testInit (t, "som", som);
  Variant eom;            testInit (t, "eom", eom);
  Variant later;          testInit (t, "later", later);
  Variant someday;        testInit (t, "someday", someday);
  Variant easter;         testInit (t, "easter", easter);
  Variant eastermonday;   testInit (t, "eastermonday", eastermonday);
  Variant ascension;      testInit (t, "ascension", ascension);
  Variant pentecost;      testInit (t, "pentecost", pentecost);
  Variant goodfriday;     testInit (t, "goodfriday", goodfriday);
  Variant pi;             testInit (t, "pi", pi);
  Variant var_true;       testInit (t, "true", var_true);
  Variant var_false;      testInit (t, "false", var_false);
  Variant midsommar;      testInit (t, "midsommar", midsommar);
  Variant midsommarafton; testInit (t, "midsommarafton", midsommarafton);
  Variant first;          testInit (t, "1st", first);
  Variant second;         testInit (t, "2nd", second);
  Variant third;          testInit (t, "3rd", third);
  Variant fourth;         testInit (t, "4th", fourth);

  // Check abbreviations.
  // TW-1515: abbreviation.minimum does not apply to date recognition
  Variant yesterday2;      testInit (t, "yesterday", yesterday2);
  Variant yesterday3;      testInit (t, "yesterda",  yesterday3);
  Variant yesterday4;      testInit (t, "yesterd",   yesterday4);
  Variant yesterday5;      testInit (t, "yester",    yesterday5);
  Variant yesterday6;      testInit (t, "yeste",     yesterday6);
  Variant yesterday7;      testInit (t, "yest",      yesterday7);
  Variant yesterday8;      testInit (t, "yes",       yesterday8);

  t.ok (now >= today,               "now >= today");
  t.ok (sod == tomorrow,            "sod == tomorrow");
  t.ok (sod > eod,                  "sod > eod");
  t.ok (yesterday < today,          "yesterday < today");
  t.ok (today < tomorrow,           "today < tomorrow");
  t.ok (socm < eocm,                "socm < eocm");
  t.ok (now < later,                "now < later");
  t.ok (now < someday,              "now < someday");
  t.ok (goodfriday < easter,        "goodfriday < easter");
  t.ok (easter < eastermonday,      "easter < eastermonday");
  t.ok (easter < midsommarafton,    "easter < midsommarafton");
  t.ok (midsommarafton < midsommar, "midsommarafton < midsommar");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
