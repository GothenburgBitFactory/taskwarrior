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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug #605 - project count zero bug?

# Setup: Add a task and complete it
qx{../src/task rc:bug.rc add One project:p1};

# Delete the task and note the completion status of the project.
my $output = qx{echo '-- y' | ../src/task rc:bug.rc 1 delete};
like ($output, qr/is 0\% complete/ms, 'Empty project correctly reported as being 0% completed.');

# Add another task, complete it and note the completion status of hte project.
qx{../src/task rc:bug.rc add Two project:p1};
$output = qx{../src/task rc:bug.rc 2 done};
like ($output, qr/is 100\% complete/ms, 'Empty project correctly reported as being 100% completed.');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

