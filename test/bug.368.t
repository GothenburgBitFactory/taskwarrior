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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/Y\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Setup: Add a recurring task, generate an instance, then add a project.
qx{../task rc:bug.rc add foo due:today recur:daily until:eom};
my $output = qx{../task rc:bug.rc info 1};

# Result: Make sure the 'until' date is rendered as a date, not an epoch.
unlike ($output, qr/Recur until\s+\d{10}/,          'until is not shown as an epoch');
  like ($output, qr/Recur until\s+\d+\/\d+\/\d{4}/, 'until is shown as a date');

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

