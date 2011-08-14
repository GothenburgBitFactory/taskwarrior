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
use Test::More tests => 12;

# Create the rc file.
if (open my $fh, '>', 'count.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'count.rc', 'Created count.rc');
}

# Test the count command.
qx{../src/task rc:count.rc add one};
qx{../src/task rc:count.rc log two};
qx{../src/task rc:count.rc add three};
qx{../src/task rc:count.rc 3 delete};
qx{../src/task rc:count.rc add four wait:eom};
qx{../src/task rc:count.rc add five due:eom recur:monthly};

my $output = qx{../src/task rc:count.rc count};
like ($output, qr/^5$/ms, 'count');

$output = qx{../src/task rc:count.rc count status:deleted};
like ($output, qr/^1$/ms, 'count status:deleted');

$output = qx{../src/task rc:count.rc count e};
like ($output, qr/^3$/ms, 'count e');

$output = qx{../src/task rc:count.rc count description.startswith:f};
like ($output, qr/^2$/ms, 'count description.startswith:f');

$output = qx{../src/task rc:count.rc count due.any:};
like ($output, qr/^1$/ms, 'count due.any:');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink 'count.rc';
ok (!-r 'count.rc', 'Removed count.rc');

exit 0;

