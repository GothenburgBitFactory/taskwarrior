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
use Test::More tests => 6;

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
my $now    = time ();
my $six    = $now -  6 * 86_400;
my $twelve = $now - 12 * 86_400;

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[uuid:"00000000-0000-0000-0000-000000000000 " status:"pending" description:"P0" entry:"$twelve"]
[uuid:"11111111-1111-1111-1111-111111111111 " status:"pending" description:"PS0" entry:"$twelve" start:"$now"]
[uuid:"22222222-2222-2222-2222-222222222222 " status:"pending" description:"PS1" entry:"$twelve" start:"$six"]
[uuid:"33333333-3333-3333-3333-333333333333 " status:"pending" description:"PS2" entry:"$twelve" start:"$twelve"]
[uuid:"44444444-4444-4444-4444-444444444444 " status:"deleted" description:"D0" entry:"$twelve" end:"$now"]
[uuid:"55555555-5555-5555-5555-555555555555 " status:"deleted" description:"D1" entry:"$twelve" end:"$six"]
[uuid:"66666666-6666-6666-6666-666666666666 " status:"deleted" description:"D2" entry:"$twelve" end:"$twelve"]
[uuid:"77777777-7777-7777-7777-777777777777 " status:"completed" description:"C0" entry:"$twelve" end:"$now"]
[uuid:"88888888-8888-8888-8888-888888888888 " status:"completed" description:"C1" entry:"$twelve" end:"$six"]
[uuid:"99999999-9999-9999-9999-999999999999 " status:"completed" description:"C2" entry:"$twelve" end:"$twelve"]
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
unlink qw(pending.data completed.data undo.data backlog.data synch.key time.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'time.rc', 'Cleanup');

exit 0;

