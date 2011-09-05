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
use Test::More tests => 14;

# Create the rc file.
if (open my $fh, '>', 'delete.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n",
            "echo.command=no\n";
  close $fh;
  ok (-r 'delete.rc', 'Created delete.rc');
}

# Add a task, delete it, undelete it.
my $output = qx{../src/task rc:delete.rc add one; ../src/task rc:delete.rc info 1};
ok (-r 'pending.data', 'pending.data created');
like ($output, qr/Status\s+Pending\n/, 'Pending');

$output = qx{../src/task rc:delete.rc 1 delete; ../src/task rc:delete.rc info 1};
like ($output, qr/Status\s+Deleted\n/, 'Deleted');
ok (-r 'completed.data', 'completed.data created');

$output = qx{echo '-- y' | ../src/task rc:delete.rc undo; ../src/task rc:delete.rc info 1};
like ($output, qr/Status\s+Pending\n/, 'Pending');
ok (-r 'completed.data', 'completed.data created');

$output = qx{../src/task rc:delete.rc 1 delete; ../src/task rc:delete.rc list};
like ($output, qr/No matches./, 'No matches');
ok (-r 'completed.data', 'completed.data created');

$output = qx{../src/task rc:delete.rc info 1};
like ($output, qr/No matches\./, 'No matches');

# Add a task, delete it, and modify on the fly.
qx{../src/task rc:delete.rc add one two};
$output = qx{../src/task rc:delete.rc list};
like ($output, qr/one two/, 'Second task added');

qx{../src/task rc:delete.rc 1 delete foo pri:H};
$output = qx{../src/task rc:delete.rc 1 info};
like ($output, qr/foo/, 'Deletion annotation successful');
like ($output, qr/H/,   'Deletion modification successful');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key delete.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'delete.rc', 'Cleanup');

exit 0;

