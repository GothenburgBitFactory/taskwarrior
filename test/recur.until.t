#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 10;

# Create the rc file.
if (open my $fh, '>', 'recur.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'recur.rc', 'Created recur.rc');
}

# Create a few recurring tasks, and test the sort order of the recur column.
qx{../src/task rc:recur.rc add foo due:now recur:2sec until:5sec};
diag ("Sleeping for 6 seconds");
sleep 6;
my $output = qx{../src/task rc:recur.rc list};
like ($output, qr/^\s+1/ms, 'Found 1');
like ($output, qr/^\s+2/ms, 'Found 2');
like ($output, qr/^\s+3/ms, 'Found 3');
like ($output, qr/^\s+4/ms, 'Found 4');

qx{../src/task rc:recur.rc do $_} for 1..5;
$output = qx{../src/task rc:recur.rc list};
like ($output, qr/and has been deleted/, 'Parent task deleted');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'recur.rc';
ok (!-r 'recur.rc', 'Removed recur.rc');

exit 0;

