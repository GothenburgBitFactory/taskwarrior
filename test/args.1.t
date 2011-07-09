#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test id before command, and id after command.
qx{../src/task rc:args.rc add one};
qx{../src/task rc:args.rc add two};
qx{../src/task rc:args.rc add three};
my $output = qx{../src/task rc:args.rc list};
like ($output, qr/one/,   'task 1 added');
like ($output, qr/two/,   'task 2 added');
like ($output, qr/three/, 'task 3 added');

$output = qx{../src/task rc:args.rc 1 done};
like ($output, qr/^Completed 1 /ms, 'COMMAND after ID');

$output = qx{../src/task rc:args.rc done 2};
like ($output, qr/^Completed 2 /ms, 'ID after COMMAND');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'args.rc';
ok (!-r 'args.rc', 'Removed args.rc');

exit 0;

