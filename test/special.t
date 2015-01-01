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
use Test::More tests => 3;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'special.rc')
{
  print $fh "data.location=.\n",
            "color.keyword.red=red\n",
            "color.alternate=\n",
            "color.tagged=\n",
            "color.pri.H=\n",
            "color.completed=\n",
            "nag=NAG\n",
            "_forcecolor=1\n";
  close $fh;
}

# Prove that +nocolor suppresses all color for a task.
qx{../src/task rc:special.rc add should have no red +nocolor priority:H 2>&1};
qx{../src/task rc:special.rc add should be red +nonag 2>&1};

my $output = qx{../src/task rc:special.rc ls 2>&1};
like ($output, qr/\s1\s+nocolor\s+should have no red/,      'no red in first task due to +nocolor');
like ($output, qr/\033\[31mshould be red\s+\033\[0m/, 'red in second task');

# Prove that +nonag suppresses nagging when a low priority task is completed
# ahead of a high priority one.
$output = qx{../src/task rc:special.rc 2 done 2>&1};
unlike ($output, qr/NAG/, '+nonag suppressed nagging for task 2');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data special.rc);
exit 0;

