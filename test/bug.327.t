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

# Setup: Add a recurring task then remove the due date.
qx{../src/task rc:$rc add foo recur:yearly due:eoy 2>&1};
qx{../src/task rc:$rc list 2>&1};
qx{../src/task rc:$rc 2 modify due: 2>&1};

# Result: Somehow the due date is incremented and wraps around to 12/31/1969,
# then keeps going back to today.
my $output = qx{../src/task rc:$rc list 2>&1};
like   ($output, qr/^1 task$/ms, "$ut: task foo shown");
unlike ($output, qr/1969/ms,     "$ut: Should not display 12/31/1969");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

