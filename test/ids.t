#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2010 - 2011, Paul Beckingham, Federico Hernandez.
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
if (open my $fh, '>', 'ids.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'ids.rc', 'Created ids.rc');
}

# Test the count command.
qx{../src/task rc:ids.rc add one    +A +B};
qx{../src/task rc:ids.rc add two    +A   };
qx{../src/task rc:ids.rc add three  +A +B};
qx{../src/task rc:ids.rc add four        };
qx{../src/task rc:ids.rc add five   +A +B};

my $output = qx{../src/task rc:ids.rc ids +A};
like ($output, qr/^1-3,5$/ms, 'ids +A --> 1-3,5');

$output = qx{../src/task rc:ids.rc ids +B};
like ($output, qr/^1,3,5$/ms, 'ids +B --> 1,3,5');

$output = qx{../src/task rc:ids.rc ids +A -B};
like ($output, qr/^2$/ms, 'ids +A -B --> 2');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'ids.rc';
ok (!-r 'ids.rc', 'Removed ids.rc');

exit 0;

