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

use strict;
use warnings;
use Test::More tests => 3;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'time.rc')
{
  print $fh "data.location=.\n";
  close $fh;
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
my $now      = time ();
my $seven    = $now -  7 * 86_400;
my $fourteen = $now - 14 * 86_400;

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[uuid:"00000000-0000-0000-0000-000000000000 " status:"pending" description:"P0" entry:"$fourteen"]
[uuid:"11111111-1111-1111-1111-111111111111 " status:"pending" description:"PS0" entry:"$fourteen" start:"$now"]
[uuid:"22222222-2222-2222-2222-222222222222 " status:"pending" description:"PS1" entry:"$fourteen" start:"$seven"]
[uuid:"33333333-3333-3333-3333-333333333333 " status:"pending" description:"PS2" entry:"$fourteen" start:"$fourteen"]
[uuid:"44444444-4444-4444-4444-444444444444 " status:"deleted" description:"D0" entry:"$fourteen" end:"$now"]
[uuid:"55555555-5555-5555-5555-555555555555 " status:"deleted" description:"D1" entry:"$fourteen" end:"$seven"]
[uuid:"66666666-6666-6666-6666-666666666666 " status:"deleted" description:"D2" entry:"$fourteen" end:"$fourteen"]
[uuid:"77777777-7777-7777-7777-777777777777 " status:"completed" description:"C0" entry:"$fourteen" end:"$now"]
[uuid:"88888888-8888-8888-8888-888888888888 " status:"completed" description:"C1" entry:"$fourteen" end:"$seven"]
[uuid:"99999999-9999-9999-9999-999999999999 " status:"completed" description:"C2" entry:"$fourteen" end:"$fourteen"]
EOF
  close $fh;
}

my $output = qx{../src/task rc:time.rc timesheet 2>&1};
like ($output, qr/Completed.+C0.+Started.+PS0/ms, 'one week of started and completed');

$output = qx{../src/task rc:time.rc timesheet 2 2>&1};
like ($output, qr/Completed.+C0.+Started.+PS0.+Completed.+C1.+Started.+PS1/ms, 'two weeks of started and completed');

$output = qx{../src/task rc:time.rc timesheet 3 2>&1};
like ($output, qr/Completed.+C0.+Started.+PS0.+Completed.+C1.+Started.+PS1.+Completed.+C2.+Started.+PS2/ms, 'three weeks of started and completed');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data time.rc);
exit 0;

