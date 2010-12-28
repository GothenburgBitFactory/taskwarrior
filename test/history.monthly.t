#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'time.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'time.rc', 'Created time.rc');
}

# Create some tasks that were started/finished in different months, then verify
# contents of the history report

my @timeArray = localtime(time);
my $now     = time ();
my $lastmonth   =  $now - $timeArray[3] * 86_400;

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[status:"pending" description:"PLW" entry:"$lastmonth" wait:"$now"]
[status:"pending" description:"PL" entry:"$lastmonth"]
[status:"deleted" description:"DLN" entry:"$lastmonth" due:"$now" end:"$now"]
[status:"deleted" description:"DLN2" entry:"$lastmonth" end:"$now"]
[status:"deleted" description:"DNN" entry:"$lastmonth" end:"$now"]
[status:"completed" description:"CLN" entry:"$lastmonth" end:"$now"]
[status:"completed" description:"CLL" entry:"$lastmonth" end:"$lastmonth"]
[status:"completed" description:"CNN" entry:"$now" end:"$now"]
[status:"completed" description:"CNN2" entry:"$now" end:"$now"]
EOF
  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

my $output = qx{../src/task rc:time.rc history.monthly};
like ($output, qr/7\s+1\s+0\s+6/, 'history.monthly - last month');
like ($output, qr/2\s+3\s+3\s+-4/, 'history.monthly - this month');
like ($output, qr/4\s+2\s+1\s+1/, 'history.monthly - average');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'time.rc';
ok (!-r 'time.rc', 'Removed time.rc');

exit 0;
