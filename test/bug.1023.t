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
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "default.project=home\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 1023: rc.default.project gets applied during modify, and should not.
qx{../src/task rc:bug.rc add foo project:garden 2>&1};
qx{../src/task rc:bug.rc add bar 2>&1};
qx{../src/task rc:bug.rc add baz rc.default.project= 2>&1};

my $output = qx{../src/task rc:bug.rc 1 info 2>&1};
like ($output, qr/Project\s*garden/, "default project not applied when otherwise specified.");

$output = qx{../src/task rc:bug.rc 2 info 2>&1};
like ($output, qr/Project\s*home/, "default project applied when blank.");

$output = qx{../src/task rc:bug.rc 3 info 2>&1};
like ($output, qr/^Description\s+baz$/m, "task baz shown.");
unlike ($output, qr/Project\s*home/, "no project applied when default project is blank.");

$output = qx{../src/task rc:bug.rc 3 modify +tag 2>&1};
like ($output, qr/^Modified 1 task.$/m, "task modified.");
unlike ($output, qr/Project\s*home/, "default project not applied on modification.");

qx{../src/task rc:bug.rc 1 modify project: 2>&1};
$output = qx{../src/task rc:bug.rc 1 info 2>&1};
like ($output, qr/^Description\s+foo$/m, "task foo shown.");
unlike ($output, qr/Project\s*garden/, "default project not re-applied on attribute removal.");
unlike ($output, qr/Project\s*home/, "default project not re-applied on attribute removal.");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

