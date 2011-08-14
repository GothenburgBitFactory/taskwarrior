#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
## All rights reserved.
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the
##
##     Free Software Foundation, Inc.,
##     51 Franklin Street, Fifth Floor,
##     Boston, MA
##     02110-1301
##     USA
##
################################################################################

use strict;
use warnings;
use Test::More tests => 17;

# Create the rc file.
if (open my $fh, '>', 'bulk.rc')
{
  print $fh "data.location=.\n",
            "confirmation=yes\n",
            "bulk=2\n";
  close $fh;
  ok (-r 'bulk.rc', 'Created bulk.rc');
}

# Add some tasks with project, prioriy and due date, some with only due date.
# Bulk add a project and priority to the tasks that were without.
qx{../src/task rc:bulk.rc add t1 pro:p1 pri:H due:monday};
qx{../src/task rc:bulk.rc add t2 pro:p1 pri:M due:tuesday};
qx{../src/task rc:bulk.rc add t3 pro:p1 pri:L due:wednesday};
qx{../src/task rc:bulk.rc add t4              due:thursday};
qx{../src/task rc:bulk.rc add t5              due:friday};
qx{../src/task rc:bulk.rc add t6              due:saturday};

my $output = qx{echo "-- quit"|../src/task rc:bulk.rc 4 5 6  modify pro:p1 pri:M};
like ($output, qr/Modified 0 tasks/, '"quit" prevents any further modifications');

$output = qx{echo "-- All"|../src/task rc:bulk.rc 4 5 6  mod pro:p1 pri:M};
unlike ($output, qr/Task 4 "t4"\n  - No changes were made/, 'Task 4 modified');
unlike ($output, qr/Task 5 "t5"\n  - No changes were made/, 'Task 5 modified');
unlike ($output, qr/Task 6 "t6"\n  - No changes were made/, 'Task 6 modified');

$output = qx{../src/task rc:bulk.rc info 4};
like ($output, qr/Project\s+p1/, 'project applied to 4');
like ($output, qr/Priority\s+M/, 'priority applied to 4');

$output = qx{../src/task rc:bulk.rc info 5};
like ($output, qr/Project\s+p1/, 'project applied to 5');
like ($output, qr/Priority\s+M/, 'priority applied to 5');

$output = qx{../src/task rc:bulk.rc info 6};
like ($output, qr/Project\s+p1/, 'project applied to 6');
like ($output, qr/Priority\s+M/, 'priority applied to 6');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink 'bulk.rc';
ok (!-r 'bulk.rc', 'Removed bulk.rc');

exit 0;

