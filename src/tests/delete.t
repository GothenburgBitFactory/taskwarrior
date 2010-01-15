#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham.
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
if (open my $fh, '>', 'delete.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n",
            "echo.command=no\n";
  close $fh;
  ok (-r 'delete.rc', 'Created delete.rc');
}

# Add a task, delete it, undelete it.
my $output = qx{../task rc:delete.rc add one; ../task rc:delete.rc info 1};
ok (-r 'pending.data', 'pending.data created');
like ($output, qr/Status\s+Pending\n/, 'Pending');

$output = qx{../task rc:delete.rc delete 1; ../task rc:delete.rc info 1};
like ($output, qr/Status\s+Deleted\n/, 'Deleted');
ok (-r 'completed.data', 'completed.data created');

$output = qx{echo 'y' | ../task rc:delete.rc undo; ../task rc:delete.rc info 1};
like ($output, qr/Status\s+Pending\n/, 'Pending');
ok (-r 'completed.data', 'completed.data created');

$output = qx{../task rc:delete.rc delete 1; ../task rc:delete.rc list};
like ($output, qr/No matches./, 'No matches');
ok (-r 'completed.data', 'completed.data created');

$output = qx{../task rc:delete.rc info 1};
like ($output, qr/Task 1 not found/, 'No matches');

# Cleanup.
ok (-r 'pending.data', 'Need to remove pending.data');
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

ok (-r 'completed.data', 'Need to remove completed.data');
unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

ok (-r 'undo.data', 'Need to remove undo.data');
unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'delete.rc';
ok (!-r 'delete.rc', 'Removed delete.rc');

exit 0;

