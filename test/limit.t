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
use Test::More tests => 4;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'limit.rc')
{
  print $fh "data.location=.\n",
            "verbose=affected\n";
  close $fh;
}

# Add a large number of tasks (> 25).
qx{../src/task rc:limit.rc add one 2>&1};
qx{../src/task rc:limit.rc add two 2>&1};
qx{../src/task rc:limit.rc add three 2>&1};
qx{../src/task rc:limit.rc add four 2>&1};
qx{../src/task rc:limit.rc add five 2>&1};
qx{../src/task rc:limit.rc add six 2>&1};
qx{../src/task rc:limit.rc add seven 2>&1};
qx{../src/task rc:limit.rc add eight 2>&1};
qx{../src/task rc:limit.rc add nine 2>&1};
qx{../src/task rc:limit.rc add ten 2>&1};
qx{../src/task rc:limit.rc add eleven 2>&1};
qx{../src/task rc:limit.rc add twelve 2>&1};
qx{../src/task rc:limit.rc add thirteen 2>&1};
qx{../src/task rc:limit.rc add fourteen 2>&1};
qx{../src/task rc:limit.rc add fifteen 2>&1};
qx{../src/task rc:limit.rc add sixteen 2>&1};
qx{../src/task rc:limit.rc add seventeen 2>&1};
qx{../src/task rc:limit.rc add eighteen 2>&1};
qx{../src/task rc:limit.rc add nineteen 2>&1};
qx{../src/task rc:limit.rc add twenty 2>&1};
qx{../src/task rc:limit.rc add twenty one 2>&1};
qx{../src/task rc:limit.rc add twenty two 2>&1};
qx{../src/task rc:limit.rc add twenty three 2>&1};
qx{../src/task rc:limit.rc add twenty four 2>&1};
qx{../src/task rc:limit.rc add twenty five 2>&1};
qx{../src/task rc:limit.rc add twenty six 2>&1};
qx{../src/task rc:limit.rc add twenty seven 2>&1};
qx{../src/task rc:limit.rc add twenty eight 2>&1};
qx{../src/task rc:limit.rc add twenty nine 2>&1};
qx{../src/task rc:limit.rc add thirty 2>&1};

my $output = qx{../src/task rc:limit.rc ls 2>&1};
like ($output, qr/^30 tasks$/ms, 'unlimited');

$output = qx{../src/task rc:limit.rc ls limit:0 2>&1};
like ($output, qr/^30 tasks$/ms, 'limited to 0 - unlimited');

$output = qx{../src/task rc:limit.rc ls limit:3 2>&1};
like ($output, qr/^30 tasks, 3 shown$/ms, 'limited to 3');

# Default height is 24 lines:
#   - header
#   - blank
#   - labels
#   - underline
#   - (data)
#   - blank
#   - affected
#   - reserved.lines
#  ------------
#   = 17 lines
$output = qx{../src/task rc:limit.rc ls limit:page 2>&1};
like ($output, qr/^30 tasks, truncated to 22 lines$/ms, 'limited to page');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data limit.rc);
exit 0;

