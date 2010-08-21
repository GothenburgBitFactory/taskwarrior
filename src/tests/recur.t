#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
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
if (open my $fh, '>', 'recur.rc')
{
  print $fh "data.location=.\n",
            "report.asc.columns=id,recur,description\n",
            "report.asc.sort=recur+\n",
            "report.desc.columns=id,recur,description\n",
            "report.desc.sort=recur-\n";
  close $fh;
  ok (-r 'recur.rc', 'Created recur.rc');
}

# Create a few recurring tasks, and test the sort order of the recur column.
qx{../task rc:recur.rc add due:tomorrow recur:daily  first};
qx{../task rc:recur.rc add due:tomorrow recur:weekly second};
qx{../task rc:recur.rc add due:tomorrow recur:3d     third};

my $output = qx{../task rc:recur.rc asc};
like ($output, qr/first .* third .* second/msx, 'daily 3d weekly');

$output = qx{../task rc:recur.rc desc};
like ($output, qr/second .* third .* first/msx, 'weekly 3d daily');

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

