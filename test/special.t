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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'special.rc')
{
  print $fh "data.location=.\n",
            "color.keyword.red=red\n",
            "color.alternate=\n",
            "color.tagged=\n",
            "color.pri.H=\n",
            "nag=NAG\n",
            "_forcecolor=1\n";
  close $fh;
  ok (-r 'special.rc', 'Created special.rc');
}

# Prove that +nocolor suppresses all color for a task.
qx{../src/task rc:special.rc add should have no red +nocolor priority:H};
qx{../src/task rc:special.rc add should be red +nonag};
my $output = qx{../src/task rc:special.rc ls};
like ($output, qr/\s1\s+H\s+should have no red/,      'no red in first task due to +nocolor');
like ($output, qr/\033\[31mshould be red\s+\033\[0m/, 'red in second task');

# Prove that +nonag suppresses nagging when a low priority task is completed
# ahead of a high priority one.
$output = qx{../src/task rc:special.rc done 2};
unlike ($output, qr/NAG/, '+nonag suppressed nagging for task 2');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'special.rc';
ok (!-r 'special.rc', 'Removed special.rc');

exit 0;

