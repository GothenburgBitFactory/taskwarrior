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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'tag.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'tag.rc', 'Created tag.rc');
}

# Add task with tags.
my $output = qx{../task rc:tag.rc add +1 This +2 is a test +3; ../task rc:tag.rc info 1};
#like ($output, qr/^Tags\s+1 2 3$/ms, 'tags found');
like ($output, qr/^Tags\s+1 2 3\n/m, 'tags found');

# Remove tags.
$output = qx{../task rc:tag.rc 1 -3 -2 -1; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '-3 -2 -1 tag removed');

# Add tags.
$output = qx{../task rc:tag.rc 1 +4 +5 +6; ../task rc:tag.rc info 1};
like ($output, qr/^Tags\s+4 5 6\n/m, 'tags found');

# Remove tags.
$output = qx{../task rc:tag.rc 1 -4 -5 -6; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '-4 -5 -6 tag removed');

# Add and remove tags.
$output = qx{../task rc:tag.rc 1 +duplicate -duplicate; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '+duplicate -duplicate NOP');

# Remove missing tag.
$output = qx{../task rc:tag.rc 1 -missing; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '-missing NOP');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pendind.data', 'Removed pending.data');

unlink 'tag.rc';
ok (!-r 'tag.rc', 'Removed tag.rc');

exit 0;

