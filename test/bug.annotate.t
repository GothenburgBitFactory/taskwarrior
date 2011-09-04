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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Attempt a blank annotation.
qx{../src/task rc:bug.rc add foo};
my $output = qx{../src/task rc:bug.rc 1 annotate};
like ($output, qr/Additional text must be provided/, 'failed on blank annotation');

# Attempt an annotation without ID
$output = qx{echo "-- n" | ../src/task rc:bug.rc annotate bar};
like ($output, qr/Command prevented from running/, 'Filter-less write command inhibited');

$output = qx{echo "-- y" | ../src/task rc:bug.rc annotate bar};
unlike ($output, qr/Command prevented from running/, 'Filter-less write command permitted');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

