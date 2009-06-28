#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2009, Paul Beckingham.
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
use Test::More tests => 28;

# Create the rc file.
if (open my $fh, '>', 'seq.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'seq.rc', 'Created seq.rc');
}

# Test sequences in done/undo
qx{../task rc:seq.rc add one mississippi};
qx{../task rc:seq.rc add two mississippi};
qx{../task rc:seq.rc do 1,2};
my $output = qx{../task rc:seq.rc info 1};
like ($output, qr/Status\s+Completed/, 'sequence do 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/Status\s+Completed/, 'sequence do 2');
qx{../task rc:seq.rc undo 1,2};
$output = qx{../task rc:seq.rc info 1};
like ($output, qr/Status\s+Pending/, 'sequence undo 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/Status\s+Pending/, 'sequence undo 2');

# Test sequences in delete/undelete
qx{../task rc:seq.rc delete 1,2};
$output = qx{../task rc:seq.rc info 1};
like ($output, qr/Status\s+Deleted/, 'sequence delete 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/Status\s+Deleted/, 'sequence delete 2');
qx{../task rc:seq.rc undelete 1,2};
$output = qx{../task rc:seq.rc info 1};
like ($output, qr/Status\s+Pending/, 'sequence undelete 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/Status\s+Pending/, 'sequence undelete 2');

# Test sequences in start/stop
qx{../task rc:seq.rc start 1,2};
$output = qx{../task rc:seq.rc info 1};
like ($output, qr/Start/, 'sequence start 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/Start/, 'sequence start 2');
qx{../task rc:seq.rc stop 1,2};
$output = qx{../task rc:seq.rc info 1};
unlike ($output, qr/Start/, 'sequence stop 1');
$output = qx{../task rc:seq.rc info 2};
unlike ($output, qr/Start/, 'sequence stop 2');

# Test sequences in modify
qx{../task rc:seq.rc 1,2 +tag};
$output = qx{../task rc:seq.rc info 1};
like ($output, qr/Tags\s+tag/, 'sequence modify 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/Tags\s+tag/, 'sequence modify 2');
qx{../task rc:seq.rc 1,2 -tag};
$output = qx{../task rc:seq.rc info 1};
unlike ($output, qr/Tags\s+tag/, 'sequence unmodify 1');
$output = qx{../task rc:seq.rc info 2};
unlike ($output, qr/Tags\s+tag/, 'sequence unmodify 2');

# Test sequences in substitutions
qx{../task rc:seq.rc 1,2 /miss/Miss/};
$output = qx{../task rc:seq.rc info 1};
like ($output, qr/Description\s+one Miss/, 'sequence substitution 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/Description\s+two Miss/, 'sequence substitution 2');

# Test sequences in info
$output = qx{../task rc:seq.rc info 1,2};
like ($output, qr/Description\s+one Miss/, 'sequence info 1');
like ($output, qr/Description\s+two Miss/, 'sequence info 2');

# Test sequences in duplicate
qx{../task rc:seq.rc duplicate 1,2 pri:H};
$output = qx{../task rc:seq.rc info 3};
like ($output, qr/Priority\s+H/, 'sequence duplicate 1');
$output = qx{../task rc:seq.rc info 4};
like ($output, qr/Priority\s+H/, 'sequence duplicate 2');

# Test sequences in annotate
qx{../task rc:seq.rc annotate 1,2 note};
$output = qx{../task rc:seq.rc info 1};
like ($output, qr/\d+\/\d+\/\d+ note/, 'sequence annotate 1');
$output = qx{../task rc:seq.rc info 2};
like ($output, qr/\d+\/\d+\/\d+ note/, 'sequence annotate 2');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'seq.rc';
ok (!-r 'seq.rc', 'Removed seq.rc');

exit 0;

