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
if (open my $fh, '>', 'limit.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'limit.rc', 'Created limit.rc');
}

# Add a large number of tasks (> 25).
qx{../src/task rc:limit.rc add one};
qx{../src/task rc:limit.rc add two};
qx{../src/task rc:limit.rc add three};
qx{../src/task rc:limit.rc add four};
qx{../src/task rc:limit.rc add five};
qx{../src/task rc:limit.rc add six};
qx{../src/task rc:limit.rc add seven};
qx{../src/task rc:limit.rc add eight};
qx{../src/task rc:limit.rc add nine};
qx{../src/task rc:limit.rc add ten};
qx{../src/task rc:limit.rc add eleven};
qx{../src/task rc:limit.rc add twelve};
qx{../src/task rc:limit.rc add thirteen};
qx{../src/task rc:limit.rc add fourteen};
qx{../src/task rc:limit.rc add fifteen};
qx{../src/task rc:limit.rc add sixteen};
qx{../src/task rc:limit.rc add seventeen};
qx{../src/task rc:limit.rc add eighteen};
qx{../src/task rc:limit.rc add nineteen};
qx{../src/task rc:limit.rc add twenty};
qx{../src/task rc:limit.rc add twenty one};
qx{../src/task rc:limit.rc add twenty two};
qx{../src/task rc:limit.rc add twenty three};
qx{../src/task rc:limit.rc add twenty four};
qx{../src/task rc:limit.rc add twenty five};
qx{../src/task rc:limit.rc add twenty six};
qx{../src/task rc:limit.rc add twenty seven};
qx{../src/task rc:limit.rc add twenty eight};
qx{../src/task rc:limit.rc add twenty nine};
qx{../src/task rc:limit.rc add thirty};

my $output = qx{../src/task rc:limit.rc ls};
like ($output, qr/^30 tasks$/ms, 'unlimited');

$output = qx{../src/task rc:limit.rc ls limit:0};
like ($output, qr/^30 tasks$/ms, 'limited to 0 - unlimited');

$output = qx{../src/task rc:limit.rc ls limit:3};
like ($output, qr/^30 tasks, 3 shown$/ms, 'limited to 3');

$output = qx{../src/task rc:limit.rc ls limit:page};
like ($output, qr/^30 tasks, truncated to 18 lines$/ms, 'limited to page');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key limit.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'limit.rc', 'Cleanup');

exit 0;

