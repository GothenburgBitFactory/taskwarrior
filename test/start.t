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
use Test::More tests => 17;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'start.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'start.rc', 'Created start.rc');
}

# Test the add/start/stop commands.
qx{../src/task rc:start.rc add one 2>&1};
qx{../src/task rc:start.rc add two 2>&1};
my $output = qx{../src/task rc:start.rc active 2>&1};
unlike ($output, qr/one/, 'one not active');
unlike ($output, qr/two/, 'two not active');

qx{../src/task rc:start.rc 1 start 2>&1};
qx{../src/task rc:start.rc 2 start 2>&1};
$output = qx{../src/task rc:start.rc active 2>&1};
like ($output, qr/one/, 'one active');
like ($output, qr/two/, 'two active');

qx{../src/task rc:start.rc 1 stop 2>&1};
$output = qx{../src/task rc:start.rc active 2>&1};
unlike ($output, qr/one/, 'one not active');
like   ($output, qr/two/, 'two active');

qx{../src/task rc:start.rc 2 stop 2>&1};
$output = qx{../src/task rc:start.rc active 2>&1};
unlike ($output, qr/one/, 'one not active');
unlike ($output, qr/two/, 'two not active');

qx{../src/task rc:start.rc 2 done 2>&1};
$output = qx{../src/task rc:start.rc list 2>&1};
unlike ($output, qr/two/, 'two deleted');

# Create the rc file.
if (open my $fh, '>', 'start2.rc')
{
  print $fh "data.location=.\n",
            "journal.time=on\n";
  close $fh;
  ok (-r 'start2.rc', 'Created start2.rc');
}

qx{../src/task rc:start2.rc 1 start 2>&1};
$output = qx{../src/task rc:start2.rc list 2>&1};
like ($output, qr/Started task/, 'one start and annotated');

qx{../src/task rc:start2.rc 1 stop 2>&1};
$output = qx{../src/task rc:start2.rc list 2>&1};
like ($output, qr/Stopped task/, 'one stopped and annotated');

# Create the rc file.
if (open my $fh, '>', 'start3.rc')
{
  print $fh "data.location=.\n",
            "journal.time=on\n",
            "journal.time.start.annotation=Nu kör vi\n",
            "journal.time.stop.annotation=Nu stannar vi\n";
  close $fh;
  ok (-r 'start3.rc', 'Created start3.rc');
}

qx{../src/task rc:start3.rc 1 start 2>&1};
$output = qx{../src/task rc:start3.rc list 2>&1};
like ($output, qr/Nu.+kör.+vi/ms, 'one start and annotated with custom description');

qx{../src/task rc:start3.rc 1 stop 2>&1};
$output = qx{../src/task rc:start3.rc list 2>&1};
like ($output, qr/Nu.+stannar.+vi/ms, 'one stopped and annotated with custom description');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data start.rc start2.rc start3.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'start.rc'       &&
    ! -r 'start2.rc'      &&
    ! -r 'start3.rc', 'Cleanup');

exit 0;

