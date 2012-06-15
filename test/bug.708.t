#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  print $fh "bulk=100\n";
  print $fh "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 708: Bad Math in Project is % Complete

# Setup: Add a few tasks
qx{../src/task rc:bug.rc add One pro:p1};
qx{../src/task rc:bug.rc add Two pro:p1};
qx{../src/task rc:bug.rc add Three pro:p1};
qx{../src/task rc:bug.rc add Four pro:p1};
qx{../src/task rc:bug.rc add Five pro:p1};
qx{../src/task rc:bug.rc add Six  pro:p1};
qx{../src/task rc:bug.rc add Seven pro:p1};
qx{../src/task rc:bug.rc add Eight pro:p1};
qx{../src/task rc:bug.rc add Nine pro:p1};
qx{../src/task rc:bug.rc add Ten pro:p1};

# Complete three tasks and ensure pending and done counts are updated correctly.
my $output = qx{../src/task rc:bug.rc 1-3 do 2>&1 >/dev/null};
like   ($output, qr/Project 'p1' is 30% complete \(7 of 10 tasks remaining\)\./ms, 'Project counts correct for a multiple done');

# Change three projects and ensure pending and done counts are updated correctly.
$output = qx{../src/task rc:bug.rc 4-6 modify pro:p2 2>&1 >/dev/null};
like   ($output, qr/Project 'p1' is 42% complete \(4 of 7 tasks remaining\)\./ms, 'Project counts correct for a multiple project reassignment part a');
like   ($output, qr/Project 'p2' is 0% complete \(3 of 3 tasks remaining\)\./ms, 'Project counts correct for a multiple project reassignment part b');

# Delete three tasks and ensure pending and done counts are updated correctly.
$output = qx{../src/task rc:bug.rc 7-9 del 2>&1 >/dev/null};
like   ($output, qr/Project 'p1' is 75% complete \(1 of 4 tasks remaining\)\./ms, 'Project counts correct for a multiple delete');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

