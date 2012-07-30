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
use Test::More tests => 10;

# Create the rc file.
if (open my $fh, '>', 'op.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'op.rc', 'Created op.rc');
}

# Setup: Add a task
qx{../src/task rc:op.rc add one   priority:H 2>&1};
qx{../src/task rc:op.rc add two   priority:M 2>&1};
qx{../src/task rc:op.rc add three priority:L 2>&1};
qx{../src/task rc:op.rc add four             2>&1};

# Test the '>=' operator.
my $output = qx{../src/task rc:op.rc ls 'priority >= M' 2>&1};
like   ($output, qr/one/,   'ls priority >= M --> one');
like   ($output, qr/two/,   'ls priority >= M --> two');
unlike ($output, qr/three/, 'ls priority >= M --> !three');
unlike ($output, qr/four/,  'ls priority >= M --> !four');

$output = qx{../src/task rc:op.rc ls 'description >= t' 2>&1};
unlike ($output, qr/one/,   'ls description >= t --> !one');
like   ($output, qr/two/,   'ls description >= t --> two');
like   ($output, qr/three/, 'ls description >= t --> three');
unlike ($output, qr/four/,  'ls description >= t --> !four');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key op.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'op.rc', 'Cleanup');

exit 0;

