#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################
##
################################################################################

use strict;
use warnings;
use Test::More tests => 81;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n",
            "color=on\n",
            "color.calendar.today=black on cyan\n",
            "color.calendar.due=black on green\n",
            "color.calendar.weeknumber=black on white\n",
            "color.calendar.overdue=black on red\n",
            "color.calendar.weekend=white on bright black\n",
            "confirmation=off\n",
            "bulk=10\n";
  close $fh;
}

my @months = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
my ($nday, $nmon, $nyear, $wday) = (localtime)[3,4,5,6];
my $day         = $nday;
my $prevmonth   = $months[($nmon-1) % 12];
my $month       = $months[($nmon) % 12];
my $nextmonth   = $months[($nmon+1) % 12];
my $prevyear    = $nyear + 1899;
my $year        = $nyear + 1900;
my $nextyear    = $nyear + 1901;

if ($day <= 9)
{
  $day = " ".$day;
}

# task calendar   and   task calendar y
my $output = qx{../src/task rc:$rc rc._forcecolor:on calendar 2>&1};
if ($wday == 6 || $wday == 0)
{
  like   ($output, qr/\[30;106m$day/,      "$ut: Current day is highlighted");
}
else
{
  like   ($output, qr/\[30;46m$day/,      "$ut: Current day is highlighted");
}
like   ($output, qr/$month\S*?\s+?$year/, "$ut: Current month and year are displayed");
$output = qx{../src/task rc:$rc add zero 2>&1};
unlike ($output, qr/\[41m\d+/,       "$ut: No overdue tasks are present");
unlike ($output, qr/\[43m\d+/,       "$ut: No due tasks are present");
$output = qx{../src/task rc:$rc rc.weekstart:Sunday calendar 2>&1};
like   ($output, qr/Su Mo Tu/,       "$ut: Week starts on Sunday");
$output = qx{../src/task rc:$rc rc.weekstart:Monday calendar 2>&1};
like   ($output, qr/Fr Sa Su/,       "$ut: Week starts on Monday");
$output = qx{../src/task rc:$rc calendar y 2>&1};
like   ($output, qr/$month\S*?\s+?$year/,         "$ut: Current month and year are displayed");
if ($month eq "Jan")
{
  $nextyear = $nextyear - 1;
}
like   ($output, qr/$prevmonth\S*?\s+?$nextyear/, "$ut: Month and year one year ahead are displayed");
if ($month eq "Jan")
{
  $nextyear = $nextyear + 1;
}
unlike ($output, qr/$month\S*?\s+?$nextyear/,     "$ut: Current month and year ahead are not displayed");

# task calendar due   and   task calendar due y
qx{../src/task rc:$rc add due:20190515 one 2>&1};
qx{../src/task rc:$rc add due:20200123 two 2>&1};
$output = qx{../src/task rc:$rc rc._forcecolor:on calendar due 2>&1};
unlike ($output, qr/April 2019/,    "$ut: April 2019 is not displayed");  # 10
like   ($output, qr/May 2019/,      "$ut: May 2019 is displayed");
unlike ($output, qr/January 2020/,  "$ut: January 2020 is not displayed");
like   ($output, qr/30;42m15/,      "$ut: Task 1 is color-coded due");
$output = qx{../src/task rc:$rc rc._forcecolor:on calendar due y 2>&1};
like   ($output, qr/30;42m23/,      "$ut: Task 2 is color-coded due");
like   ($output, qr/April 2020/,    "$ut: April 2020 is displayed");
unlike ($output, qr/May 2020/,      "$ut: May 2020 is not displayed");
qx{../src/task rc:$rc ls 2>&1};
qx{../src/task rc:$rc 1-3 del 2>&1};
qx{../src/task rc:$rc add due:20080408 three 2>&1};
$output = qx{../src/task rc:$rc rc._forcecolor:on calendar due 2>&1};
like   ($output, qr/April 2008/,     "$ut: April 2008 is displayed");
like   ($output, qr/41m 8/,          "$ut: Task 3 is color-coded overdue");
like   ($output, qr/37;100m19/,      "$ut: Saturday April 19, 2008 is color-coded");
like   ($output, qr/37;100m20/,      "$ut: Sunday April 20, 2008 is color-coded");  # 20
like   ($output, qr/30;47m  1/,      "$ut: Weeknumbers are color-coded");

# task calendar 2016
$output = qx{../src/task rc:$rc rc.weekstart:Monday calendar 2016 2>&1};
unlike ($output, qr/2015/,           "$ut: Year 2015 is not displayed");
unlike ($output, qr/2017/,           "$ut: Year 2017 is not displayed");
like   ($output, qr/January 2016/,   "$ut: January 2016 is displayed");
like   ($output, qr/December 2016/,  "$ut: December 2016 is displayed");
like   ($output, qr/53 +1/,          "$ut: 2015 has 53 weeks (ISO)");
like   ($output, qr/1 +4/,           "$ut: First week in 2016 starts with Mon Jan 4 (ISO)");
like   ($output, qr/52 +26/,         "$ut: Last week in 2016 starts with Mon Dec 26 (ISO)");
like   ($output, qr/9 +29/,          "$ut: Leap year - Feb 29 is Monday in week 9 (ISO)");
$output = qx{../src/task rc:$rc rc.weekstart:Sunday calendar 2016 2>&1};
like   ($output, qr/1 +1/,           "$ut: First week in 2016 starts with Fri Jan 1 (US)");  # 30
like   ($output, qr/53 +25/,         "$ut: Last week in 2016 starts with Sun Dec 25 (US)");
$output = qx{../src/task rc:$rc rc.weekstart:Monday rc.displayweeknumber:off calendar 2016 2>&1};
unlike ($output, qr/53/,             "$ut: Weeknumbers are not displayed");

# task calendar 4 2010
$output = qx{../src/task rc:$rc rc.monthsperline:1 calendar 4 2010 2>&1};
unlike ($output, qr/March 2010/,     "$ut: March 2010 is not displayed");
like   ($output, qr/April 2010/,     "$ut: April 2010 is displayed");
unlike ($output, qr/May 2010/,       "$ut: May 2010 is not displayed");

# calendar offsets
$output = qx{../src/task rc:$rc rc.calendar.offset:on rc.monthsperline:1 calendar 1 2011 2>&1};
unlike ($output, qr/November 2010/,  "$ut: November 2010 is not displayed");
like   ($output, qr/December 2010/,  "$ut: December 2010 is displayed");
unlike ($output, qr/January 2011/,   "$ut: January  2011 is not displayed");
$output = qx{../src/task rc:$rc rc.calendar.offset:on rc.calendar.offset.value:2 rc.monthsperline:1 calendar 1 2011 2>&1};
unlike ($output, qr/January 2011/,   "$ut: January  2011 is not displayed");
unlike ($output, qr/February 2011/,  "$ut: February 2011 is not displayed");  # 40
like   ($output, qr/March 2011/,     "$ut: March 2011 is displayed");
unlike ($output, qr/April 2011/,     "$ut: April 2011 is not displayed");
$output = qx{../src/task rc:$rc rc.calendar.offset:on rc.calendar.offset.value:-12 rc.monthsperline:1 calendar 2>&1};
like   ($output, qr/$month\S*?\s+?$prevyear/, "$ut: Current month and year ahead are displayed");
unlike ($output, qr/$month\S*?\s+?$year/,     "$ut: Current month and year are not displayed");
$output = qx{../src/task rc:$rc rc.calendar.offset:on rc.calendar.offset.value:12 rc.monthsperline:1 calendar 2>&1};
unlike ($output, qr/$month\S*?\s+?$year/,     "$ut: Current month and year are not displayed");
like   ($output, qr/$month\S*?\s+?$nextyear/, "$ut: Current month and year ahead are displayed");

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', "$ut: Removed pending.data");
unlink 'undo.data';
ok (!-r 'undo.data', "$ut: Removed undo.data");
unlink $rc;
ok (!-r $rc, "$ut: Removed $rc");

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n",
            "dateformat.holiday=YMD\n",
            "dateformat.report=YMD\n",
            "calendar.details=full\n",
            "calendar.details.report=list\n",
            "calendar.holidays=full\n",
            "color=on\n",
            "color.alternate=\n",
            "color.calendar.weekend=\n",
            "color.calendar.holiday=black on bright yellow\n",
            "confirmation=no\n",
            "holiday.AAAA.name=AAAA\n",
            "holiday.AAAA.date=20150101\n",
            "holiday.BBBBBB.name=BBBBBB\n",
            "holiday.BBBBBB.date=20150115\n",
            "holiday.åäö.name=åäö\n",
            "holiday.åäö.date=20150125\n",
            "verbose=blank,header,footnote,label,new-id,affected,edit,special,project,sync\n";
  close $fh;
}

# task calendar details
qx{../src/task rc:$rc add due:20150105 one 2>&1};
qx{../src/task rc:$rc add due:20150110 two 2>&1};
qx{../src/task rc:$rc add due:20150210 three 2>&1};
qx{../src/task rc:$rc add due:20150410 four 2>&1};
qx{../src/task rc:$rc add due:20151225 five 2>&1};
qx{../src/task rc:$rc add due:20141231 six 2>&1};
qx{../src/task rc:$rc add due:20160101 seven 2>&1};
qx{../src/task rc:$rc add due:20081231 eight 2>&1};

$output = qx{../src/task rc:$rc rc.calendar.legend:no calendar 2>&1};
unlike ($output, qr/Legend:/,      "$ut: Legend is not displayed");  # 50

$output = qx{../src/task rc:$rc calendar rc.monthsperline:3 1 2015 2>&1};
like   ($output, qr/January 2015/, "$ut: January 2015 is displayed");
like   ($output, qr/20150105/,     "$ut: Due date 20150105 is displayed");
like   ($output, qr/20150110/,     "$ut: Due date 20150110 is displayed");
like   ($output, qr/20150210/,     "$ut: Due date 20150210 is displayed");
unlike ($output, qr/20141231/,     "$ut: Due date 20141231 is not displayed");
unlike ($output, qr/20150410/,     "$ut: Due date 20150410 is not displayed");
like   ($output, qr/3 tasks/,      "$ut: 3 due tasks are displayed");

$output = qx{../src/task rc:$rc calendar due 2>&1};
like   ($output, qr/December 2008/, "$ut: December 2008 is displayed");
like   ($output, qr/20081231/,      "$ut: Due date 20081231 is displayed");
like   ($output, qr/1 task/,        "$ut: 1 due task is displayed");  # 60

$output = qx{../src/task rc:$rc calendar 2015 2>&1};
like   ($output, qr/January 2015/,  "$ut: January 2015 is displayed");
like   ($output, qr/December 2015/, "$ut: December 2015 is displayed");
unlike ($output, qr/20141231/,      "$ut: Due date 20141231 is not displayed");
unlike ($output, qr/20160101/,      "$ut: Due date 20160101 is not displayed");
like   ($output, qr/5 tasks/,       "$ut: 5 due tasks are displayed");

$day = $nday;
$day = "0".$day if $day < 10;

my $mon = $nmon + 1;
$mon = "0".$mon if $mon < 10;
my $duedate = $year.$mon.$day;

qx{../src/task rc:$rc add due:$duedate nine 2>&1};
$output = qx{../src/task rc:$rc calendar rc.monthsperline:1 2>&1};
like   ($output, qr/$month\S*?\s+?$year/, "$ut: Current month and year are displayed");
like   ($output, qr/$duedate/,            "$ut: Due date on current day is displayed");
like   ($output, qr/[12] task/,           "$ut: 1/2 due task(s) are displayed");

$output = qx{../src/task rc:$rc calendar rc.monthsperline:1 1 2015 2>&1};
like   ($output, qr/Date/,         "$ut: Word Date is displayed");
like   ($output, qr/Holiday/,      "$ut: Word Holiday is displayed");  # 70
like   ($output, qr/20150101/,     "$ut: Holiday 20150101 is displayed");
like   ($output, qr/20150115/,     "$ut: Holiday 20150115 is displayed");
like   ($output, qr/20150125/,     "$ut: Holiday 20150125 is displayed");
like   ($output, qr/AAAA/,         "$ut: Holiday name AAAA is displayed");
like   ($output, qr/BBBBBB/,       "$ut: Holiday name BBBBBB is displayed");
like   ($output, qr/åäö/,          "$ut: Holiday name åäö is displayed");

$output = qx{../src/task rc:$rc calendar rc._forcecolor:on rc.monthsperline:1 rc.calendar.details:sparse rc.calendar.holidays:sparse 1 2015 2>&1};
unlike ($output, qr/Date/,         "$ut: Word Date is not displayed");
unlike ($output, qr/Holiday/,      "$ut: Word Holiday is not displayed");
like   ($output, qr/30;103m 1/,    "$ut: Holiday AAAA is color-coded");
like   ($output, qr/30;103m15/,    "$ut: Holiday BBBBBB is color-coded");  # 80
like   ($output, qr/30;103m25/,    "$ut: Holiday åäö is color-coded");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
