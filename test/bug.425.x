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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', '425.rc')
{
  print $fh "data.location=.";

  close $fh;
  ok (-r '425.rc', 'Created 425.rc');
}

# Bug #425: Parser preventing editing of an existing task depending on description

# Create a task and attempt to revise the description to include the word 'in'
# (this breaks in 1.9.3 and earlier)

qx{../src/task rc:425.rc add Foo};
qx{../src/task rc:425.rc 1 Bar in Bar};

my $output = qx{../src/task rc:425.rc 1 ls};

like ($output, qr/1\s+Bar in Bar/m, 'parser - interpret \'in\' in description');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink '425.rc';
ok (!-r '425.rc', 'Removed 425.rc');

exit 0;
