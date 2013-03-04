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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "confirmation=on\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 955: When deleting the child of a recurring task, taskwarrior does not ask
#          whether it should delete the parent.
qx{../src/task rc:bug.rc add foo due:now recur:1day 2>&1};
my $output = qx{../src/task rc:bug.rc ls 2>&1};
like ($output, qr/^2 tasks/ms, '2 child tasks created');

qx{printf 'y\nn\n' | ../src/task rc:bug.rc 2 delete 2>&1};
$output = qx{../src/task rc:bug.rc ls 2>&1};
like ($output, qr/^1 task/ms, '1 child task remaining');

qx{printf 'y\ny\n' | ../src/task rc:bug.rc 2 delete 2>&1};
$output = qx{../src/task rc:bug.rc ls 2>&1};
like ($output, qr/^No matches/ms, 'No tasks remaining');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

