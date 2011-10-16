#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 13;

# Create the rc file.
if (open my $fh, '>', 'wait.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";

  close $fh;
  ok (-r 'wait.rc', 'Created wait.rc');
}

# Add a waiting task, check it is not there, wait, then check it is.
qx{../src/task rc:wait.rc add yeswait wait:2s};
qx{../src/task rc:wait.rc add nowait};

my $output = qx{../src/task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task visible');
unlike ($output, qr/yeswait/ms, 'waiting task invisible');

diag ("3 second delay");
sleep 3;

$output = qx{../src/task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task still visible');
like ($output, qr/yeswait/ms, 'waiting task now visible');

qx{../src/task rc:wait.rc 1 modify wait:2s};
$output = qx{../src/task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task visible');
unlike ($output, qr/yeswait/ms, 'waiting task invisible');

diag ("3 second delay");
sleep 3;

$output = qx{../src/task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task still visible');
like ($output, qr/yeswait/ms, 'waiting task now visible');

qx{../src/task rc:wait.rc add wait:tomorrow tomorrow};
$output = qx{../src/task rc:wait.rc ls};
unlike ($output, qr/tomorrow/ms, 'waiting task invisible');

$output = qx{../src/task rc:wait.rc all status:waiting wait:tomorrow};
like ($output, qr/tomorrow/ms, 'waiting task visible when specifically queried');

$output = qx{../src/task rc:wait.rc add Complain due:today wait:tomorrow};
like ($output, qr/Warning: You have specified a 'wait' date that is after the 'due' date\./, 'warning on wait after due');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key wait.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'wait.rc', 'Cleanup');

exit 0;
