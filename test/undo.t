#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 11;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'undo.rc')
{
  print $fh "data.location=.\n",
            "echo.command=no\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'undo.rc', 'Created undo.rc');
}

# Test the add/do/undo commands.
my $output = qx{../src/task rc:undo.rc add one 2>&1; ../src/task rc:undo.rc info 1 2>&1};
ok (-r 'pending.data', 'pending.data created');
like ($output, qr/Status\s+Pending\n/, 'Pending');

$output = qx{../src/task rc:undo.rc 1 do 2>&1; ../src/task rc:undo.rc info 1 2>&1};
ok (-r 'completed.data', 'completed.data created');
like ($output, qr/Status\s+Completed\n/, 'Completed');

$output = qx{../src/task rc:undo.rc undo 2>&1; ../src/task rc:undo.rc info 1 2>&1};
ok (-r 'completed.data', 'completed.data created');
like ($output, qr/Status\s+Pending\n/, 'Pending');

$output = qx{../src/task rc:undo.rc 1 do 2>&1; ../src/task rc:undo.rc list 2>&1 >/dev/null};
like ($output, qr/No matches/, 'No matches');

# Bug 1060: Test that if undo is given an argument it catches and reports the correct error.
$output = qx{../src/task rc:undo.rc undo 1 2>&1};
unlike ($output, qr/Unknown error/, 'No unknown error');
like ($output, qr/The undo command does not allow further task modification/, 'Correct error caught and reported');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data undo.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'undo.rc', 'Cleanup');

exit 0;
