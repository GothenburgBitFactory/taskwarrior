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
if (open my $fh, '>', 'bug_sort.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug_sort.rc', 'Created bug_sort.rc');
}

my $setup = "../task rc:bug_sort.rc add one;"
          . "../task rc:bug_sort.rc add two;"
          . "../task rc:bug_sort.rc add three recur:daily due:eom;";
qx{$setup};

my $output = qx{../task rc:bug_sort.rc list};
like ($output, qr/three.*(?:one.*two|two.*one)/msi, 'list did not hang');

qx{../task rc:bug_sort.rc 1 priority:H};
$output = qx{../task rc:bug_sort.rc list};
like ($output, qr/three.*one.*two/msi, 'list did not hang after pri:H on 1');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bug_sort.rc';
ok (!-r 'bug_sort.rc', 'Removed bug_sort.rc');

exit 0;

