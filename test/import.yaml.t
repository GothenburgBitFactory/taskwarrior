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
use Test::More tests => 15;

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
    entry: 1234567889
  task:
    uuid: 11111111-1111-1111-1111-111111111111
    description: one
    project: B
    status: pending
    entry: 1234567889
  task:
    uuid: 22222222-2222-2222-2222-222222222222
    description: two
    status: completed
    entry: 1234524689
    end: 1234524690
...
EOF

  close $fh;
  ok (-r 'import.txt', 'Created sample import data');
}

my $output = qx{../src/task rc:import.rc import import.txt};
like ($output, qr/Imported 3 tasks successfully./, 'no errors');
# Imported 3 tasks successfully.

$output = qx{../src/task rc:import.rc list};
# ID Project Pri Due Active Age     Description
# -- ------- --- --- ------ ------- -----------
#  1 A                      1.5 yrs zero
#  2 B                      1.5 yrs one
# 
# 2 tasks

like   ($output, qr/1.+A.+zero/, 't1 present');
like   ($output, qr/2.+B.+one/,  't2 present');
unlike ($output, qr/3.+two/,     't3 missing');

$output = qx{../src/task rc:import.rc completed};
# Complete  Project Pri Age     Description
# --------- ------- --- ------- -----------
# 2/13/2009             1.5 yrs two
# 
# 1 task

unlike ($output, qr/1.+A.+zero/,       't1 missing');
unlike ($output, qr/2.+B.+one/,        't2 missing');
like   ($output, qr/2\/13\/2009.+two/, 't3 present');

# Make sure that a duplicate task cannot be imported.
$output = qx{../src/task rc:import.rc import import.txt};
like ($output, qr/Cannot add task because the uuid .+ is not unique\./, 'error on duplicate uuid');

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

