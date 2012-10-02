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
if (open my $fh, '>', 'recur.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'recur.rc', 'Created recur.rc');
}

# Create a few recurring tasks, and test the sort order of the recur column.
qx{../src/task rc:recur.rc add foo due:now recur:2sec until:5sec 2>&1};
diag ("Sleeping for 6 seconds");
sleep 6;
my $output = qx{../src/task rc:recur.rc list 2>&1};
like ($output, qr/^\s+2/ms, 'Found 2');
like ($output, qr/^\s+3/ms, 'Found 3');
like ($output, qr/^\s+4/ms, 'Found 4');
like ($output, qr/^\s+5/ms, 'Found 5');

qx{../src/task rc:recur.rc 2 do 2>&1};
qx{../src/task rc:recur.rc 3 do 2>&1};
qx{../src/task rc:recur.rc 4 do 2>&1};
qx{../src/task rc:recur.rc 5 do 2>&1};
$output = qx{../src/task rc:recur.rc list 2>&1 >/dev/null};
like ($output, qr/and was deleted/, 'Parent task deleted');

$output = qx{../src/task rc:recur.rc diag 2>&1};
like ($output, qr/No duplicates found/, 'No duplicate UUIDs detected');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data recur.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'recur.rc', 'Cleanup');

exit 0;

