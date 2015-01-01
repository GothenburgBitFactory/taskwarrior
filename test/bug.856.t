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
use Test::More tests => 6;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n";
  close $fh;
}

# Bug 856: "task list project.none:" does not work.
# Note: Not using "assigned" and "unassigned" because one is a subset of the
# other.
qx{../src/task rc:$rc add assigned project:X 2>&1};
qx{../src/task rc:$rc add floating 2>&1};

my $output = qx{../src/task rc:$rc ls project: 2>&1};
like   ($output, qr/floating/, "$ut: project:  matches floating");
unlike ($output, qr/assigned/, "$ut: project:  does not match assigned");

$output = qx{../src/task rc:$rc ls project:'' 2>&1};
like   ($output, qr/floating/, "$ut: project:\'\'  matches floating");
unlike ($output, qr/assigned/, "$ut: project:\'\'  does not match assigned");

$output = qx{../src/task rc:$rc ls project.none: 2>&1};
like   ($output, qr/floating/, "$ut: project.none:  matches floating");
unlike ($output, qr/assigned/, "$ut: project.none:  does not match assigned");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

