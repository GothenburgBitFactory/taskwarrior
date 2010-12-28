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
if (open my $fh, '>', 'datesort.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n",
            "report.small_list.description=Small list\n",
            "report.small_list.columns=due,description\n",
            "report.small_list.labels=Due,Description\n",
            "report.small_list.sort=due+\n",
            "report.small_list.filter=status:pending\n",
            "report.small_list.dateformat=MD\n";

  close $fh;
  ok (-r 'datesort.rc', 'Created datesort.rc');
}

qx{../src/task rc:datesort.rc add two   due:20100201};
qx{../src/task rc:datesort.rc add one   due:20100101};
qx{../src/task rc:datesort.rc add three due:20100301};

my $output = qx{../src/task rc:datesort.rc small_list};
like ($output, qr/one.+two.+three/ms, 'Sorting by due+ with format MD works');

$output = qx{../src/task rc:datesort.rc rc.report.small_list.sort=due- small_list};
like ($output, qr/three.+two.+one/ms, 'Sorting by due- with format MD works');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'datesort.rc';
ok (!-r 'datesort.rc', 'Removed datesort.rc');

exit 0;

