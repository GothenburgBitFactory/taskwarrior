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
use Test::More tests => 12;

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

# Bug 956 - 'task ids' prints the header, which prevents using the command in
# external script (it applies also for 'uuids' and helper subcommands).

qx{../src/task rc:$rc add test 2>&1};

# Solution 1: rc.verbose=nothing
my $output = qx{TASKRC=$rc ../src/task rc:$rc rc.verbose=nothing ids 2>&1};
like   ($output, qr/^1$/m,          "$ut: ID 1 shown");
unlike ($output, qr/TASKRC/ms,      "$ut: The header does not appear with 'ids' (rc.verbose=nothing)");

$output = qx{TASKRC=$rc ../src/task rc.verbose=nothing uuids 2>&1};
like   ($output, qr/^[0-9a-f-]*$/m, "$ut: UUID shown");
unlike ($output, qr/TASKRC/ms,      "$ut: The header does not appear with 'uuids' (rc.verbose=nothing)");

$output = qx{TASKRC=$rc ../src/task rc.verbose=nothing uuids 2>&1};
like   ($output, qr/^[0-9a-f-]*$/m, "$ut: UUID shown");
unlike ($output, qr/TASKRC/ms,      "$ut: The header does not appear with 'uuids' (rc.verbose=nothing)");

# Solution 2: task ... 2>/dev/null
$output = qx{TASKRC=$rc ../src/task rc:$rc ids 2>/dev/null};
like   ($output, qr/^1$/m,          "$ut: ID 1 shown");
unlike ($output, qr/TASKRC/ms,      "$ut: The header does not appear with 'ids' (2>/dev/null)");

$output = qx{TASKRC=$rc ../src/task _ids 2>/dev/null};
like   ($output, qr/^[0-9a-f-]*$/m, "$ut: UUID shown");
unlike ($output, qr/TASKRC/ms,      "$ut: The header does not appear with '_ids' (2>/dev/null)");

$output = qx{TASKRC=$rc ../src/task _ids 2>/dev/null};
like   ($output, qr/^[0-9a-f-]*$/m, "$ut: UUID shown");
unlike ($output, qr/TASKRC/ms,      "$ut: The header does not appear with '_ids' (2>/dev/null)");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
