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
use POSIX qw(mktime);
use Test::More tests => 5;

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
            "confirmation=off\n";
  close $fh;
}

# Test the count command.
qx{../src/task rc:$rc add one 2>&1};
qx{../src/task rc:$rc log two 2>&1};
qx{../src/task rc:$rc add three 2>&1};
qx{../src/task rc:$rc 2 delete 2>&1};
qx{../src/task rc:$rc add four wait:eom 2>&1};
qx{../src/task rc:$rc add five due:eom recur:monthly 2>&1};

my $output = qx{../src/task rc:$rc count 2>&1};
like ($output, qr/^5\n/ms, "$ut: count");

$output = qx{../src/task rc:$rc count status:deleted rc.debug:1 2>&1};
like ($output, qr/^1\n/ms, "$ut: count status:deleted");

$output = qx{../src/task rc:$rc count e 2>&1};
like ($output, qr/^3\n/ms, "$ut: count e");

$output = qx{../src/task rc:$rc count description.startswith:f 2>&1};
like ($output, qr/^2\n/ms, "$ut: count description.startswith:f");

$output = qx{../src/task rc:$rc count due.any: 2>&1};
like ($output, qr/^1\n/ms, "$ut: count due.any:");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

