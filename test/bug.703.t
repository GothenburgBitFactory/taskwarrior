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
use Test::More tests => 10;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  print $fh "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 703: /from/t/g fails to make all changes to annotations

# Setup: Add a few tasks
qx{../src/task rc:bug.rc add This is a test 2>&1};
qx{../src/task rc:bug.rc 1 annotate Annotation one 2>&1};
qx{../src/task rc:bug.rc 1 annotate Annotation two 2>&1};
qx{../src/task rc:bug.rc 1 annotate Annotation three 2>&1};

my $output = qx{../src/task rc:bug.rc long 2>&1};
like ($output, qr/This is a test/,   'original description');
like ($output, qr/Annotation one/,   'original annotation one');
like ($output, qr/Annotation two/,   'original annotation two');
like ($output, qr/Annotation three/, 'original annotation three');

qx{../src/task rc:bug.rc 1 modify /i/I/g 2>&1};
$output = qx{../src/task rc:bug.rc long 2>&1};
like ($output, qr/ThIs Is a test/,   'new description');
like ($output, qr/AnnotatIon one/,   'new annotation one');
like ($output, qr/AnnotatIon two/,   'new annotation two');
like ($output, qr/AnnotatIon three/, 'new annotation three');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

