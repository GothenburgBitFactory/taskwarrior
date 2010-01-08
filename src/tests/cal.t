#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
## All rights reserved.
##
## Unit test cal.t originally writen by Federico Hernandez
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the
##
##     Free Software Foundation, Inc.,
##     51 Franklin Street, Fifth Floor,
##     Boston, MA
##     02110-1301
##     USA
##
################################################################################

use strict;
use warnings;
use Test::More tests => 60;

# Create the rc file.
if (open my $fh, '>', 'cal.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n",
            "color=on\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'cal.rc', 'Created cal.rc');
}

my @months = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
my ($nday, $nmon, $nyear) = (localtime)[3,4,5];
my $day         = $nday;
my $prevmonth   = $months[($nmon-1) % 12];
my $month       = $months[($nmon) % 12];
my $nextmonth   = $months[($nmon+1) % 12];
my $year        = $nyear + 1900;
my $nextyear    = $nyear + 1901;

if ( $day <= 9)
{
  $day = " ".$day;
}

# task cal   and   task cal y
my $output = qx{../task rc:cal.rc rc._forcecolor:on cal};
like   ($output, qr/\[30;46m$day/,      'Current day is highlighted');
like   ($output, qr/$month\w+?\s+?$year/, 'Current month and year are displayed');
$output = qx{../task rc:cal.rc add zero};
unlike ($output, qr/\[41m\d+/,       'No overdue tasks are present');
unlike ($output, qr/\[43m\d+/,       'No due tasks are present');
$output = qx{../task rc:cal.rc rc.weekstart:Sunday cal};
like   ($output, qr/Su Mo Tu/,     'Week starts on Sunday'); 
$output = qx{../task rc:cal.rc rc.weekstart:Monday cal};
like   ($output, qr/Fr Sa Su/,     'Week starts on Monday'); 
$output = qx{../task rc:cal.rc cal y};
like   ($output, qr/$month\w+?\s+?$year/,         'Current month and year are displayed');
if ( $month eq "Jan")
{
  $nextyear = $nextyear - 1;
}
like   ($output, qr/$prevmonth\w+?\s+?$nextyear/, 'Month and year one year ahead are displayed');
if ( $month eq "Jan")
{
  $nextyear = $nextyear + 1;
}
unlike ($output, qr/$month\w+?\s+?$nextyear/,     'Current month and year ahead are not displayed');

# task cal due   and   task cal due y
qx{../task rc:cal.rc add due:20190515 one};
qx{../task rc:cal.rc add due:20200123 two};
$output = qx{../task rc:cal.rc rc._forcecolor:on cal due};
unlike ($output, qr/April 2019/,   'April 2019 is not displayed');
like   ($output, qr/May 2019/,     'May 2019 is displayed');
unlike ($output, qr/January 2020/, 'January 2020 is not displayed');
like   ($output, qr/30;42m15/,      'Task 1 is color-coded due');
$output = qx{../task rc:cal.rc rc._forcecolor:on cal due y};
like   ($output, qr/30;42m23/,      'Task 2 is color-coded due');
like   ($output, qr/April 2020/,   'April 2020 is displayed');
unlike ($output, qr/May 2020/,     'May 2020 is not displayed');
qx{../task rc:cal.rc ls};
qx{../task rc:cal.rc del 1-3};
qx{../task rc:cal.rc add due:20080408 three};
$output = qx{../task rc:cal.rc rc._forcecolor:on cal due};
like   ($output, qr/April 2008/, 'April 2008 is displayed');
like   ($output, qr/41m 8/,      'Task 3 is color-coded overdue');
like   ($output, qr/30;47m19/,   'Saturday April 19, 2008 is color-coded');
like   ($output, qr/30;47m20/,   'Sunday April 20, 2008 is color-coded');

# task cal 2016
$output = qx{../task rc:cal.rc rc.weekstart:Monday cal 2016};
unlike ($output, qr/2015/,           'Year 2015 is not displayed');
unlike ($output, qr/2017/,           'Year 2017 is not displayed');
like   ($output, qr/January 2016/,   'January 2016 is displayed');
like   ($output, qr/December 2016/,  'December 2016 is displayed');
like   ($output, qr/53 +1/,          '2015 has 53 weeks (ISO)');
like   ($output, qr/1 +4/,           'First week in 2016 starts with Mon Jan 4 (ISO)');
like   ($output, qr/52 +26/,         'Last week in 2016 starts with Mon Dec 26 (ISO)');
like   ($output, qr/9 +29/,          'Leap year - Feb 29 is Monday in week 9 (ISO)');
$output = qx{../task rc:cal.rc rc.weekstart:Sunday cal 2016};
like   ($output, qr/1 +1/,           'First week in 2016 starts with Fri Jan 1 (US)');
like   ($output, qr/53 +25/,         'Last week in 2016 starts with Sun Dec 25 (US)');
$output = qx{../task rc:cal.rc rc.weekstart:Monday rc.displayweeknumber:off cal 2016};
unlike ($output, qr/53/,             'Weeknumbers are not displayed');

# task cal 4 2010
$output = qx{../task rc:cal.rc rc.monthsperline:1 cal 4 2010};
unlike ($output, qr/March 2010/,   'March 2010 is not displayed');
like   ($output, qr/April 2010/,   'April 2010 is displayed');
unlike ($output, qr/May 2010/,     'May 2010 is not displayed');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');
unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');
unlink 'cal.rc';
ok (!-r 'cal.rc', 'Removed cal.rc');

# Create the rc file.
if (open my $fh, '>', 'details.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n",
            "calendar.details=yes\n",
            "calendar.details.report=list\n",
            "color=on\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'details.rc', 'Created details.rc');
}

# task calendar details
qx{../task rc:details.rc add due:20150105 one};
qx{../task rc:details.rc add due:20150110 two};
qx{../task rc:details.rc add due:20150210 three};
qx{../task rc:details.rc add due:20150410 four};
qx{../task rc:details.rc add due:20151225 five};
qx{../task rc:details.rc add due:20141231 six};
qx{../task rc:details.rc add due:20160101 seven};
qx{../task rc:details.rc add due:20081231 eight};

$output = qx{../task rc:details.rc cal rc.monthsperline:3 1 2015};
like   ($output, qr/January 2015/, 'January 2015 is displayed');
like   ($output, qr/20150105/,     'Due date 20150105 is displayed');
like   ($output, qr/20150110/,     'Due date 20150110 is displayed');
like   ($output, qr/20150210/,     'Due date 20150210 is displayed');
unlike ($output, qr/20141231/,     'Due date 20141231 is not displayed');
unlike ($output, qr/20150410/,     'Due date 20150410 is not displayed');
like   ($output, qr/3 tasks/,      '3 due tasks are displayed');

$output = qx{../task rc:details.rc cal due};
like   ($output, qr/December 2008/, 'December 2008 is displayed');
like   ($output, qr/20081231/,      'Due date 20081231 is displayed');
like   ($output, qr/1 task/,        '1 due task is displayed');

$output = qx{../task rc:details.rc cal 2015};
like   ($output, qr/January 2015/,  'January 2015 is displayed');
like   ($output, qr/December 2015/, 'December 2015 is displayed');
unlike ($output, qr/20141231/,      'Due date 20141231 is not displayed');
unlike ($output, qr/20160101/,      'Due date 20160101 is not displayed');
like   ($output, qr/5 tasks/,       '5 due tasks are displayed');

$day = $nday;
if ( $day <= 9)
{
  $day = "0".$day;
}
my $mon = $nmon + 1;
if ( $mon <= 9)
{
  $mon = "0".$mon;
}
my $duedate = $year.$mon.$day;

qx{../task rc:details.rc add due:$duedate rc.monthsperline:1 nine};
$output = qx{../task rc:details.rc cal};
like   ($output, qr/$month\w+?\s+?$year/, 'Current month and year are displayed');
like   ($output, qr/$duedate/,            'Due date on current day is displayed');
like   ($output, qr/1 task/,              '1 due task is displayed');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');
unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');
unlink 'details.rc';
ok (!-r 'details.rc', 'Removed details.rc');

exit 0;
