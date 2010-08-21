#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
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
if (open my $fh, '>', 'tag.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'tag.rc', 'Created tag.rc');
}

# Add task with tags.
my $output = qx{../task rc:tag.rc add +one This +two is a test +three; ../task rc:tag.rc info 1};
like ($output, qr/^Tags\s+one two three\n/m, 'tags found');

# Remove tags.
$output = qx{../task rc:tag.rc 1 -three -two -one; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '-three -two -one tag removed');

# Add tags.
$output = qx{../task rc:tag.rc 1 +four +five +six; ../task rc:tag.rc info 1};
like ($output, qr/^Tags\s+four five six\n/m, 'tags found');

# Remove tags.
$output = qx{../task rc:tag.rc 1 -four -five -six; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '-four -five -six tag removed');

# Add and remove tags.
$output = qx{../task rc:tag.rc 1 +duplicate -duplicate; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '+duplicate -duplicate NOP');

# Remove missing tag.
$output = qx{../task rc:tag.rc 1 -missing; ../task rc:tag.rc info 1};
unlike ($output, qr/^Tags/m, '-missing NOP');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'tag.rc';
ok (!-r 'tag.rc', 'Removed tag.rc');

exit 0;

