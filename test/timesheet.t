#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 11;

# Create the rc file.
if (open my $fh, '>', 'time.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'time.rc', 'Created time.rc');
}

# Create some tasks that were started/finished in different weeks, then verify
# the presence and grouping in the timesheet report.
#   P0   pending, this week
#   PS0  started, this week
#   PS1  started, last week
#   PS2  started, 2wks ago
#   D0   deleted, this week
#   D1   deleted, last week
#   D2   deleted, 2wks ago
#   C0   completed, this week
#   C1   completed, last week
#   C2   completed, 2wks ago
my $now     = time ();
my $seven   = $now -  7 * 86_400;
my $fourteen = $now - 14 * 86_400;

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[status:"pending" description:"P0" entry:"$fourteen"]
[status:"pending" description:"PS0" entry:"$fourteen" start:"$now"]
[status:"pending" description:"PS1" entry:"$fourteen" start:"$seven"]
[status:"pending" description:"PS2" entry:"$fourteen" start:"$fourteen"]
[status:"deleted" description:"D0" entry:"$fourteen" end:"$now"]
[status:"deleted" description:"D1" entry:"$fourteen" end:"$seven"]
[status:"deleted" description:"D2" entry:"$fourteen" end:"$fourteen"]
[status:"completed" description:"C0" entry:"$fourteen" end:"$now"]
[status:"completed" description:"C1" entry:"$fourteen" end:"$seven"]
[status:"completed" description:"C2" entry:"$fourteen" end:"$fourteen"]
EOF
  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

my $output = qx{../src/task rc:time.rc timesheet};
like ($output, qr/Completed.+C0.+Started.+PS0/ms, 'one week of started and completed');

$output = qx{../src/task rc:time.rc timesheet 2};
like ($output, qr/Completed.+C0.+Started.+PS0.+Completed.+C1.+Started.+PS1/ms, 'two weeks of started and completed');

$output = qx{../src/task rc:time.rc timesheet 3};
like ($output, qr/Completed.+C0.+Started.+PS0.+Completed.+C1.+Started.+PS1.+Completed.+C2.+Started.+PS2/ms, 'three weeks of started and completed');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink 'time.rc';
ok (!-r 'time.rc', 'Removed time.rc');

exit 0;

