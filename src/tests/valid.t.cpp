////////////////////////////////////////////////////////////////////////////////
// Copyright 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <library.h>
#include <UnitTest.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (66);

  t.notok (validUnsignedInteger (""),    "!validUnsignedInteger ''");
  t.notok (validUnsignedInteger ("F"),   "!validUnsignedInteger 'F'");
  t.notok (validUnsignedInteger (" "),   "!validUnsignedInteger ' '");
  t.ok (validUnsignedInteger    ("0"),   "validUnsignedInteger '0'");
  t.notok (validUnsignedInteger ("-1"),  "!validUnsignedInteger '-1'");
  t.ok (validUnsignedInteger    ("+1"),  "validUnsignedInteger '+1'");
  t.ok (validUnsignedInteger    ("123"), "validUnsignedInteger '123'");

  t.notok (validInteger (""),    "!validInteger ''");
  t.notok (validInteger ("F"),   "!validInteger 'F'");
  t.notok (validInteger (" "),   "!validInteger ' '");
  t.ok (validInteger    ("0"),   "validInteger '0'");
  t.ok (validInteger    ("-1"),  "validInteger '-1'");
  t.ok (validInteger    ("+1"),  "validInteger '+1'");
  t.ok (validInteger    ("123"), "validInteger '123'");

  t.notok (validReal (""),   "!validReal ''");
  t.notok (validReal ("F"),  "!validReal 'F'");
  t.notok (validReal (" "),  "!validReal ' '");
  t.ok (validReal ("0"),     "validReal '0'");
  t.ok (validReal ("1"),     "validReal '1'");
  t.ok (validReal ("-1"),    "validReal '-1'");
  t.ok (validReal ("1.23"),  "validReal '1.23'");
  t.ok (validReal ("-1.23"), "validReal '-1.23'");

  //  0/1, T/F, t/f, true/false, True/False, TRUE/FALSE, -/+, yes/no
  t.notok (validBoolean (""),   "!validBoolean ''");
  t.notok (validBoolean (" "),  "!validBoolean ' '");
  t.notok (validBoolean ("x"),  "!validBoolean 'x'");
  t.notok (validBoolean ("2"),  "!validBoolean '1'");
  t.ok (validBoolean ("0"),     "validBoolean '0'");
  t.ok (validBoolean ("1"),     "validBoolean '1'");
  t.ok (validBoolean ("T"),     "validBoolean 'T'");
  t.ok (validBoolean ("F"),     "validBoolean 'F'");
  t.ok (validBoolean ("t"),     "validBoolean 't'");
  t.ok (validBoolean ("f"),     "validBoolean 'f'");
  t.ok (validBoolean ("true"),  "validBoolean 'true'");
  t.ok (validBoolean ("false"), "validBoolean 'false'");
  t.ok (validBoolean ("True"),  "validBoolean 'True'");
  t.ok (validBoolean ("False"), "validBoolean 'False'");
  t.ok (validBoolean ("TRUE"),  "validBoolean 'TRUE'");
  t.ok (validBoolean ("FALSE"), "validBoolean 'FALSE'");
  t.ok (validBoolean ("-"),     "validBoolean '-'");
  t.ok (validBoolean ("+"),     "validBoolean '+'");
  t.ok (validBoolean ("YES"),   "validBoolean 'YES'");
  t.ok (validBoolean ("NO"),    "validBoolean 'NO'");
  t.ok (validBoolean ("Yes"),   "validBoolean 'Yes'");
  t.ok (validBoolean ("No"),    "validBoolean 'No'");
  t.ok (validBoolean ("yes"),   "validBoolean 'yes'");
  t.ok (validBoolean ("no"),    "validBoolean 'no'");
  t.ok (validBoolean ("Y"),     "validBoolean 'Y'");
  t.ok (validBoolean ("N"),    "validBoolean 'N'");
  t.ok (validBoolean ("y"),    "validBoolean 'y'");
  t.ok (validBoolean ("n"),    "validBoolean 'n'");
  t.ok (validBoolean ("on"),    "validBoolean 'on'");
  t.ok (validBoolean ("off"),   "validBoolean 'off'");
  t.ok (validBoolean ("On"),    "validBoolean 'On'");
  t.ok (validBoolean ("Off"),   "validBoolean 'Off'");
  t.ok (validBoolean ("ON"),    "validBoolean 'ON'");
  t.ok (validBoolean ("OFF"),   "validBoolean 'OFF'");

  t.ok    (validColor ("black"),   "validColor black");
  t.ok    (validColor ("red"),     "validColor red");
  t.ok    (validColor ("green"),   "validColor green");
  t.ok    (validColor ("yellow"),  "validColor yellow");
  t.ok    (validColor ("blue"),    "validColor blue");
  t.ok    (validColor ("magenta"), "validColor magenta");
  t.ok    (validColor ("cyan"),    "validColor cyan");
  t.ok    (validColor ("white"),   "validColor white");
  t.ok    (validColor (""),        "validColor nocolor");
  t.notok (validColor ("donkey"),  "fail <- validColor donkey");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

