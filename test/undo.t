#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 19;

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
            "confirmation=no\n";
  close $fh;
}

# Test the add/done/undo commands.
my $output = qx{../src/task rc:$rc add one 2>&1; ../src/task rc:$rc info 1 2>&1};
ok (-r 'pending.data', "$ut: pending.data created");
ok (! -r 'completed.data', "$ut: completed.data not created");
like ($output, qr/Status\s+Pending\n/, "$ut: Pending");

$output = qx{../src/task rc:$rc 1 done 2>&1; ../src/task rc:$rc info 1 2>&1};
ok (! -r 'completed.data', "$ut: completed.data created");
like ($output, qr/Status\s+Completed\n/, "$ut: Completed");

$output = qx{../src/task rc:$rc undo 2>&1; ../src/task rc:$rc info 1 2>&1};
ok (-r 'completed.data', "$ut: completed.data created");
like ($output, qr/Status\s+Pending\n/, "$ut: Pending");

$output = qx{../src/task rc:$rc 1 done 2>&1; ../src/task rc:$rc list 2>&1 >/dev/null};
like ($output, qr/No matches/, "$ut: No matches");

# Bug 1060: Test that if undo is given an argument it catches and reports the correct error.
$output = qx{../src/task rc:$rc undo 1 2>&1};
unlike ($output, qr/Unknown error/, "$ut: No unknown error");
like ($output, qr/The undo command does not allow further task modification/, "$ut: Correct error caught and reported");

# Add a new task and undo it.
$output = qx{../src/task rc:$rc add two 2>&1; ../src/task rc:$rc info 1 2>&1};
unlike ($output, qr/Unknown error/, "$ut: No unknown error");
like ($output, qr/Status\s+Pending\n/, "$ut: Pending");

$output = qx{../src/task rc:$rc undo 2>&1; ../src/task rc:$rc info 1 2>&1};
like ($output, qr/Task removed\.\n/, "$ut: Task removed");
like ($output, qr/No matches\.\n/, "$ut: No matches");

# Inspect backlog.data
if (open my $fh, '<', 'backlog.data')
{
  my @lines = <$fh>;
  close $fh;

  diag ($_) for @lines;
  is (scalar (@lines), 4, "$ut: 4 lines of backlog");
  ok (index ($lines[0], '"status":"pending"')   != -1, "$ut: [0] pending");
  ok (index ($lines[1], '"status":"completed"') != -1, "$ut: [1] completed");
  ok (index ($lines[2], '"status":"pending"')   != -1, "$ut: [2] pending");
  ok (index ($lines[3], '"status":"completed"') != -1, "$ut: [3] completed");
}
else
{
  fail ("$ut: 4 lines of backlog");
  fail ("$ut: [0] pending");
  fail ("$ut: [1] completed");
  fail ("$ut: [2] pending");
  fail ("$ut: [3] completed");
}

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
