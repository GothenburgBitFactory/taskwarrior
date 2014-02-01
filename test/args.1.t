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
use Test::More tests => 8;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test id before command, and id after command.
qx{../src/task rc:args.rc add one 2>&1};
qx{../src/task rc:args.rc add two 2>&1};
qx{../src/task rc:args.rc add three 2>&1};
my $output = qx{../src/task rc:args.rc list 2>&1};
like ($output, qr/one/,   'task 1 added');
like ($output, qr/two/,   'task 2 added');
like ($output, qr/three/, 'task 3 added');

$output = qx{../src/task rc:args.rc 1 done 2>&1};
like ($output, qr/^Completed 1 task.$/ms, 'COMMAND after ID');

$output = qx{../src/task rc:args.rc done 2 2>&1};
like ($output, qr/^Command prevented from running.$/ms, 'ID after COMMAND');
unlike ($output, qr/^Completed 1 task.$/ms, 'ID after COMMAND');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data args.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'args.rc', 'Cleanup');

exit 0;

