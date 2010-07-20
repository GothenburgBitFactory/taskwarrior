#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham.
## All rights reserved.
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
use Test::More tests => 12;

# Create the rc file.
if (open my $fh, '>', 'countdown.rc')
{
  print $fh "data.location=.\n";
  print $fh "report.countdown.description=Countdown report\n";
  print $fh "report.countdown.columns=id,countdown,description\n";
  print $fh "report.countdown.labels=ID,Countdown,Description\n";
  print $fh "report.countdown.sort=countdown-"; 

  close $fh;
  ok (-r 'countdown.rc', 'Created countdown.rc');
}

# Create a variety of pending tasks with increasingly higher due dates
# and ensure sort order.

my @timeArray = localtime(time);
my $now     = time ();

my $nowminusone   =  $now - 3600;
my $nowplusone   =  $now + 3600;
my $nowplusseven   =  $now + 7 * 3600;
my $nowplustwelve   =  $now + 12 * 3600;
my $nowplusthirtysix   =  $now + 36 * 3600;
my $nowplusseventytwo   =  $now + 72 * 3600;

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[status:"pending" description:"Due one hour ago" entry:"$now" due:"$nowminusone"]
[status:"pending" description:"Due now" entry:"$now" due:"$now"]
[status:"pending" description:"Due one hour in the future" entry:"$now" due:"$nowplusone"]
[status:"pending" description:"Due seven hours in the future" entry:"$now" due:"$nowplusseven"]
[status:"pending" description:"Due twelve hours in the future" entry:"$now" due:"$nowplustwelve"]
[status:"pending" description:"Due thirty-six hours in the future" entry:"$now" due:"$nowplusthirtysix"]
[status:"pending" description:"Due seventy-two hours in the future" entry:"$now" due:"$nowplusseventytwo"]

EOF
  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

my $output = qx{../task rc:countdown.rc countdown};
like ($output, qr/\s-3 days\s.*\s-1 day\s/s, 'countdown - oldest first');
like ($output, qr/\s-1 day\s.*\s-12 hrs\s/s, 'countdown - next second oldest');
like ($output, qr/\s-12 hrs\s.*\s-7 hrs\s/s, 'countdown - next third oldest');
like ($output, qr/\s-7 hrs\s.*\s-1 hr\s/s, 'countdown - next fourth oldest');
like ($output, qr/\s-1 hr\s.*\s- Due now\s/s, 'countdown - next fifth oldest');
like ($output, qr/\s- Due now\s.*\s1 hr\s/s, 'countdown - next sixth oldest');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'countdown.rc';
ok (!-r 'countdown.rc', 'Removed countdown.rc');

exit 0;
