#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 5;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
}

# Test the -- argument.
qx{../src/task rc:args.rc add project:p pri:H +tag foo 2>&1};
my $output = qx{../src/task rc:args.rc info 1 2>&1};
like ($output, qr/Description\s+foo\n/ms, 'task add project:p pri:H +tag foo');

qx{../src/task rc:args.rc 1 modify project:p pri:H +tag -- foo 2>&1};
$output = qx{../src/task rc:args.rc info 1 2>&1};
like ($output, qr/Description\s+foo\n/ms, 'task 1 modify project:p pri:H +tag -- foo');

qx{../src/task rc:args.rc 1 modify project:p pri:H -- +tag foo 2>&1};
$output = qx{../src/task rc:args.rc info 1 2>&1};
like ($output, qr/Description\s+\+tag\sfoo\n/ms, 'task 1 modify project:p pri:H -- +tag foo');

qx{../src/task rc:args.rc 1 modify project:p -- pri:H +tag foo 2>&1};
$output = qx{../src/task rc:args.rc info 1 2>&1};
like ($output, qr/Description\s+pri:H\s\+tag\sfoo\n/ms, 'task 1 modify project:p -- pri:H +tag foo');

qx{../src/task rc:args.rc 1 modify -- project:p pri:H +tag foo 2>&1};
$output = qx{../src/task rc:args.rc info 1 2>&1};
like ($output, qr/Description\s+project:p\spri:H\s\+tag\sfoo\n/ms, 'task 1 modify -- project:p pri:H +tag foo');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data args.rc);
exit 0;

