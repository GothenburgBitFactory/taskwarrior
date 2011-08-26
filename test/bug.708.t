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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  print $fh "bulk=100\n";
  print $fh "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 708: Bad Math in Project is % Complete

# Setup: Add a few tasks
qx{../src/task rc:bug.rc add One pro:p1};
qx{../src/task rc:bug.rc add Two pro:p1};
qx{../src/task rc:bug.rc add Three pro:p1};
qx{../src/task rc:bug.rc add Four pro:p1};
qx{../src/task rc:bug.rc add Five pro:p1};
qx{../src/task rc:bug.rc add Six  pro:p1};
qx{../src/task rc:bug.rc add Seven pro:p1};
qx{../src/task rc:bug.rc add Eight pro:p1};
qx{../src/task rc:bug.rc add Nine pro:p1};
qx{../src/task rc:bug.rc add Ten pro:p1};

# Complete three tasks and ensure pending and done counts are updated correctly.
my $output = qx{../src/task rc:bug.rc 1-3 do};
like   ($output, qr/Project 'p1' is 30% complete \(7 of 10 tasks remaining\)\./ms, 'Project counts correct for a multiple done');

# Change three projects and ensure pending and done counts are updated correctly.
$output = qx{../src/task rc:bug.rc 4-6 modify pro:p2};
like   ($output, qr/Project 'p1' is 42% complete \(4 of 7 tasks remaining\)\./ms, 'Project counts correct for a multiple project reassignment part a');
like   ($output, qr/Project 'p2' is 0% complete \(3 of 3 tasks remaining\)\./ms, 'Project counts correct for a multiple project reassignment part b');

# Delete three tasks and ensure pending and done counts are updated correctly.
$output = qx{../src/task rc:bug.rc 7-9 del};
like   ($output, qr/Project 'p1' is 75% complete \(1 of 4 tasks remaining\)\./ms, 'Project counts correct for a multiple delete');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

