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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'count.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'count.rc', 'Created count.rc');
}

# Test the count command.
qx{../src/task rc:count.rc add one};
qx{../src/task rc:count.rc log two};
qx{../src/task rc:count.rc add three};
qx{../src/task rc:count.rc 2 delete};
qx{../src/task rc:count.rc add four wait:eom};
qx{../src/task rc:count.rc add five due:eom recur:monthly};

my $output = qx{../src/task rc:count.rc count};
like ($output, qr/^5\n/ms, 'count');

$output = qx{../src/task rc:count.rc count status:deleted rc.debug:1};
like ($output, qr/^1\n/ms, 'count status:deleted');

$output = qx{../src/task rc:count.rc count e};
like ($output, qr/^3\n/ms, 'count e');

$output = qx{../src/task rc:count.rc count description.startswith:f};
like ($output, qr/^2\n/ms, 'count description.startswith:f');

$output = qx{../src/task rc:count.rc count due.any:};
like ($output, qr/^1\n/ms, 'count due.any:');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key count.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'count.rc', 'Cleanup');

exit 0;

