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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'sorting.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'sorting.rc', 'Created sorting.rc');
}

# Test sort order for most reports (including list) of:
#   due+,priority-,active+,project+
qx{../task rc:sorting.rc add due:eow priority:H project:A due:tomorrow one};
qx{../task rc:sorting.rc add due:eow priority:H project:B due:tomorrow two};
qx{../task rc:sorting.rc add due:eow priority:H project:C due:tomorrow three};
qx{../task rc:sorting.rc add due:eow priority:H project:D due:tomorrow four};
my $output = qx{../task rc:sorting.rc list};
like ($output, qr/one.+two.+three.+four/ms, 'no active task sorting');

qx{../task rc:sorting.rc start 1};
qx{../task rc:sorting.rc start 3};
$output = qx{../task rc:sorting.rc list};
like ($output, qr/one.+three.+two.+four/ms, 'active task sorting');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'sorting.rc';
ok (!-r 'sorting.rc', 'Removed sorting.rc');

exit 0;

