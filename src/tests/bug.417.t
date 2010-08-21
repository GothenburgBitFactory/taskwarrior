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
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "defaultwidth=100\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug #417: Sorting by countdown_compact not working
qx{../task rc:bug.rc add due:yesterday before};
qx{../task rc:bug.rc add due:today     now};
qx{../task rc:bug.rc add due:tomorrow  after};

my $output = qx{../task rc:bug.rc rc.report.long.sort:countdown+ long};
like ($output, qr/before.+now.+after/ms, 'rc.report.long.sort:countdown+ works');

$output = qx{../task rc:bug.rc rc.report.long.sort:countdown- long};
like ($output, qr/after.+now.+before/ms, 'rc.report.long.sort:countdown- works');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bug.rc';
ok (!-r 'bug.rc', 'Removed bug.rc');

exit 0;

