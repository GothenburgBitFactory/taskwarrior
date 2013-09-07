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
use Test::More tests => 12;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 932: Modifying recurring task's recurrence period - strange outcome
# - add a recurring task with multiple child tasks
# - modify a child task and test for propagation
# - modify the parent task and test for propagation
qx{../src/task rc:bug.rc add R due:yesterday recur:daily 2>&1};
my $output = qx{../src/task rc:bug.rc list 2>&1};
like ($output, qr/2.+R/ms, 'Found child 0');
like ($output, qr/3.+R/ms, 'Found child 1');
like ($output, qr/4.+R/ms, 'Found child 2');

qx{echo 'y' | ../src/task rc:bug.rc 2 mod project:P 2>&1};
$output = qx{../src/task rc:bug.rc list 2>&1};
like ($output, qr/2\s+P.+R/ms, 'Found modified child 0');
like ($output, qr/3\s+P.+R/ms, 'Found modified child 1 (propagated from 0)');
like ($output, qr/4\s+P.+R/ms, 'Found modified child 2 (propagated from 0)');

qx{echo 'y' | ../src/task rc:bug.rc 1 mod priority:H 2>&1};
$output = qx{../src/task rc:bug.rc list 2>&1};
like ($output, qr/2\s+P.+H.+R/ms, 'Found modified child 0 (propagated from parent');
like ($output, qr/3\s+P.+H.+R/ms, 'Found modified child 1 (propagated from parent)');
like ($output, qr/4\s+P.+H.+R/ms, 'Found modified child 2 (propagated from parent)');

$output = qx{../src/task rc:bug.rc diag 2>&1};
like ($output, qr/No duplicates found/, 'No duplicate UUIDs detected');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

