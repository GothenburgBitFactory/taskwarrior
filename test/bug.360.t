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
use Test::More tests => 10;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Setup: Add a recurring task, generate an instance, then add a project.
qx{../src/task rc:bug.rc add foo due:tomorrow recur:daily 2>&1};
qx{../src/task rc:bug.rc ls 2>&1};

# Result: trying to add the project generates an error about removing
# recurrence from a task.
my $output = qx{echo 'y' | ../src/task rc:bug.rc 1 modify project:bar 2>&1};
like ($output, qr/^Modified 2 tasks.$/ms, '2 tasks modified');
unlike ($output, qr/^You cannot remove the recurrence from a recurring task.$/ms, 'No recurrence removal error');

# Now try to generate the error above via regular means - ie, is it actually
# doing what it should?
# TODO Removing recur: from a recurring task should also remove imask and parent.
$output = qx{../src/task rc:bug.rc 2 modify recur: 2>&1 >/dev/null};
like ($output, qr/^You cannot remove the recurrence from a recurring task.$/ms, 'Recurrence removal error');

# Prevent removal of the due date from a recurring task.
# TODO Removing due: from a recurring task should also remove recur, imask and parent
$output = qx{../src/task rc:bug.rc 2 modify due: 2>&1 >/dev/null};
like ($output, qr/^You cannot remove the due date from a recurring task.$/ms, 'Cannot remove due date from a recurring task');

# Allow removal of the due date from a non-recurring task.
qx{../src/task rc:bug.rc add nonrecurring due:today 2>&1};
$output = qx{../src/task rc:bug.rc ls 2>&1};
like ($output, qr/^2 task.$/ms, '2 tasks shown');
my ($id) = $output =~ /(\d+)\s+nonrecurring/;
$output = qx{../src/task rc:bug.rc $id modify due: 2>&1};
like ($output, qr/^Modified 1 task.$/ms, 'no task modified');
unlike ($output, qr/^You cannot remove the due date from a recurring task.$/ms, 'Can remove due date from a non-recurring task');

$output = qx{../src/task rc:bug.rc diag 2>&1};
like ($output, qr/^\s+No duplicates found$/m, 'No duplicate UUIDs detected');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

