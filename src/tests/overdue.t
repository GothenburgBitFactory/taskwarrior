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
if (open my $fh, '>', 'due.rc')
{
  print $fh "data.location=.\n",
            "due=4\n";
  close $fh;
  ok (-r 'due.rc', 'Created due.rc');
}

# Add an overdue task, a due task, and a regular task.  The "overdue" report
# should list only the one task.
qx{../task rc:due.rc add due:yesterday one};
qx{../task rc:due.rc add due:tomorrow two};
qx{../task rc:due.rc add due:eoy three};
my $output = qx{../task rc:due.rc overdue};
like   ($output, qr/one/,   'overdue: task 1 shows up');
unlike ($output, qr/two/,   'overdue: task 2 does not show up');
unlike ($output, qr/three/, 'overdue: task 3 does not show up');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'due.rc';
ok (!-r 'due.rc', 'Removed due.rc');

exit 0;

