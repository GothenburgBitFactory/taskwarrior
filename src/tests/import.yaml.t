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
use Test::More tests => 11;

# Create the rc file.
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'import.rc', 'Created import.rc');
}

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh <<EOF;
%YAML 1.1
---
  task:
    uuid: 00000000-0000-0000-0000-000000000000
    description: zero
    project: A
    status: pending
  task:
    uuid: 11111111-1111-1111-1111-111111111111
    description: one
    project: B
    status: pending
  task:
    uuid: 22222222-2222-2222-2222-222222222222
    description: two
    status: completed
    end: 1234567890
...
EOF

  close $fh;
  ok (-r 'import.txt', 'Created sample import data');
}

my $output = qx{../task rc:import.rc import import.txt};
like ($output, qr/Imported 3 tasks successfully./, 'no errors');

$output = qx{../task rc:import.rc list};
like ($output, qr/1.+A.+zero/, 't1');
like ($output, qr/2.+B.+one/,   't2');

$output = qx{../task rc:import.rc completed};
like ($output, qr/2\/13\/2009.+two/,  't3');

# Cleanup.
unlink 'import.txt';
ok (!-r 'import.txt', 'Removed import.txt');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'import.rc';
ok (!-r 'import.rc', 'Removed import.rc');

exit 0;

