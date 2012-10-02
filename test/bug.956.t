#! /usr/bin/env perl
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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 956 - 'task ids' prints the header, which prevents using the command in
# external script (it applies also for 'uuids' and helper subcommands).

qx{../src/task rc:bug.rc add test 2>&1};

# Solution 1: rc.verbose=nothing
my $output = qx{TASKRC=bug.rc ../src/task rc:bug.rc rc.verbose=nothing ids 2>&1};
unlike ($output, qr/TASKRC/ms, 'The header does not appear with "ids" (rc.verbose=nothing)');

$output = qx{TASKRC=bug.rc ../src/task rc.verbose=nothing uuids 2>&1};
unlike ($output, qr/TASKRC/ms, 'The header does not appear with "uuids" (rc.verbose=nothing)');

$output = qx{TASKRC=bug.rc ../src/task rc.verbose=nothing uuids 2>&1};
unlike ($output, qr/TASKRC/ms, 'The header does not appear with "uuids" (rc.verbose=nothing)');

# Solution 2: task ... 2>/dev/null
$output = qx{TASKRC=bug.rc ../src/task rc:bug.rc ids 2>/dev/null};
unlike ($output, qr/TASKRC/ms, 'The header does not appear with "ids" (2>/dev/null)');

$output = qx{TASKRC=bug.rc ../src/task _ids 2>/dev/null};
unlike ($output, qr/TASKRC/ms, 'The header does not appear with "_ids" (2>/dev/null)');

$output = qx{TASKRC=bug.rc ../src/task _ids 2>/dev/null};
unlike ($output, qr/TASKRC/ms, 'The header does not appear with "_ids" (2>/dev/null)');

### Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;
