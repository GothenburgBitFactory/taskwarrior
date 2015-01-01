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
use Test::More tests => 8;

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

# Test id before command, and id after command.
qx{../src/task rc:$rc add one 2>&1};
qx{../src/task rc:$rc add two 2>&1};
qx{../src/task rc:$rc add three 2>&1};
my $output = qx{../src/task rc:$rc list 2>&1};
like ($output, qr/one/,                                 "$ut: task 1 added");
like ($output, qr/two/,                                 "$ut: task 2 added");
like ($output, qr/three/,                               "$ut: task 3 added");

$output = qx{../src/task rc:$rc 1 done 2>&1};
like ($output, qr/^Completed 1 task\.$/ms,              "$ut: COMMAND after ID");

$output = qx{../src/task rc:$rc rc.allow.empty.filter:yes done 2 2>&1};
like ($output, qr/^Command prevented from running.$/ms, "$ut: ID after COMMAND, allowing empty filter");
unlike ($output, qr/^Completed 1 task\.$/ms,            "$ut: ID after COMMAND, allowing empty filter");

$output = qx{../src/task rc:$rc rc.allow.empty.filter:no done 2 2>&1};
like ($output, qr/^You did not specify a filter, and with the 'allow\.empty\.filter' value, no action is taken\.$/ms,
                                                        "$ut: ID after COMMAND, disallowing empty filter");
unlike ($output, qr/^Completed 1 task\.$/ms,            "$ut: ID after COMMAND, disallowing empty filter");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

