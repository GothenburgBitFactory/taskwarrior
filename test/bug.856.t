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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 856: "task list project.none:" does not work.
# Note: Not using "assigned" and "unassigned" because one is a subset of the
# other.
qx{../src/task rc:bug.rc add assigned project:X};
qx{../src/task rc:bug.rc add floating};

my $output = qx{../src/task rc:bug.rc ls project:};
like   ($output, qr/floating/, 'project:  matches floating');
unlike ($output, qr/assigned/, 'project:  does not match assigned');

$output = qx{../src/task rc:bug.rc ls project:''};
like   ($output, qr/floating/, 'project:\'\'  matches floating');
unlike ($output, qr/assigned/, 'project:\'\'  does not match assigned');

$output = qx{../src/task rc:bug.rc ls project.none:};
like   ($output, qr/floating/, 'project.none:  matches floating');
unlike ($output, qr/assigned/, 'project.none:  does not match assigned');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

