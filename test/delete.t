#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 16;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'delete.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'delete.rc', 'Created delete.rc');
}

# Add a task, delete it, undelete it.
my $output = qx{../src/task rc:delete.rc add one 2>&1; ../src/task rc:delete.rc info 1 2>&1};
ok (-r 'pending.data', 'pending.data created');
like ($output, qr/Status\s+Pending\n/, 'Pending');

$output = qx{../src/task rc:delete.rc 1 delete 2>&1; ../src/task rc:delete.rc info 1 2>&1};
like ($output, qr/Status\s+Deleted\n/, 'Deleted');
ok (-r 'completed.data', 'completed.data created');

$output = qx{../src/task rc:delete.rc undo 2>&1; ../src/task rc:delete.rc info 1 2>&1};
like ($output, qr/Status\s+Pending\n/, 'Pending');
ok (-r 'completed.data', 'completed.data created');

$output = qx{../src/task rc:delete.rc 1 delete 2>&1; ../src/task rc:delete.rc list 2>&1 >/dev/null};
like ($output, qr/No matches./, 'No matches');
ok (-r 'completed.data', 'completed.data created');

$output = qx{../src/task rc:delete.rc info 1 2>&1 >/dev/null};
like ($output, qr/No matches\./, 'No matches');  # 10

# Add a task, delete it, and modify on the fly.
qx{../src/task rc:delete.rc add one two 2>&1};
$output = qx{../src/task rc:delete.rc list 2>&1};
like ($output, qr/one two/, 'Second task added');

qx{../src/task rc:delete.rc 1 delete foo pri:H 2>&1};
$output = qx{../src/task rc:delete.rc 1 info 2>&1};
like ($output, qr/foo/, 'Deletion annotation successful');
like ($output, qr/H/,   'Deletion modification successful');

# Add a task, complete it, then delete it.
qx{../src/task rc:delete.rc add three 2>&1};
$output = qx{../src/task rc:delete.rc 2 info 2>&1};
like ($output, qr/three/, 'added and verified new task');
my ($uuid) = $output =~ /UUID\s+(\S+)/;
qx{../src/task rc:delete.rc 2 done 2>&1};
qx{../src/task rc:delete.rc $uuid delete 2>&1};
$output = qx{../src/task rc:delete.rc $uuid info 2>&1};
like ($output, qr/Deleted/, 'task added, completed, then deleted');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data delete.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'delete.rc', 'Cleanup');

exit 0;

