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
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
}

# Add a task, delete it, undelete it.
my $output = qx{../src/task rc:$rc add one 2>&1; ../src/task rc:$rc info 1 2>&1};
ok (-r 'pending.data',                 "$ut: pending.data created");
like ($output, qr/Status\s+Pending\n/, "$ut: Pending");

$output = qx{../src/task rc:$rc 1 delete 2>&1; ../src/task rc:$rc info 1 2>&1};
like ($output, qr/Status\s+Deleted\n/, "$ut: Deleted");

$output = qx{../src/task rc:$rc undo 2>&1; ../src/task rc:$rc info 1 2>&1};
like ($output, qr/Status\s+Pending\n/, "$ut: Pending");

$output = qx{../src/task rc:$rc 1 delete 2>&1; ../src/task rc:$rc list 2>&1 >/dev/null};
like ($output, qr/No matches./,        "$ut: No matches");
ok (-r 'completed.data',               "$ut: completed.data created");

$output = qx{../src/task rc:$rc info 1 2>&1 >/dev/null};
like ($output, qr/No matches\./,       "$ut: No matches");  # 10

# Add a task, delete it, and modify on the fly.
qx{../src/task rc:$rc add one two 2>&1};
$output = qx{../src/task rc:$rc list 2>&1};
like ($output, qr/one two/,            "$ut: Second task added");

qx{../src/task rc:$rc 1 delete foo pri:H 2>&1};
$output = qx{../src/task rc:$rc 1 info 2>&1};
like ($output, qr/foo/,                "$ut: Deletion annotation successful");
like ($output, qr/H/,                  "$ut: Deletion modification successful");

# Add a task, complete it, then delete it.
qx{../src/task rc:$rc add three 2>&1};
$output = qx{../src/task rc:$rc 2 info 2>&1};
like ($output, qr/three/,              "$ut: added and verified new task");
my ($uuid) = $output =~ /UUID\s+(\S+)/;
qx{../src/task rc:$rc 2 done 2>&1};
qx{../src/task rc:$rc $uuid delete 2>&1};
$output = qx{../src/task rc:$rc $uuid info 2>&1};
like ($output, qr/Deleted/,            "$ut: task added, completed, then deleted");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

