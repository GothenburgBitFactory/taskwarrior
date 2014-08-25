#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 2;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
}

# Bug 656: Recurring task continually spawns new tasks

# Setup: Add a pair of recurring tasks
qx{../src/task rc:$rc add First recurring task due:tomorrow rec:daily 2>&1};
qx{../src/task rc:$rc add Second recurring task due:tomorrow rec:daily 2>&1};

# Bug describes that each time task is executed, another child task of the
# Second recurring task is created.
qx{../src/task rc:$rc ls 2>&1};

# Result: Check for an extra task the second time task ls is executed
my $output = qx{../src/task rc:$rc ls 2>&1};
like   ($output, qr/^2 tasks$/ms,                        "$ut: 2 tasks shown.");
unlike ($output, qr/^\s3\s*Second recurring task\s*$/ms, "$ut: Extra task detected.");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

