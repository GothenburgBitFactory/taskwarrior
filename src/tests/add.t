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
use Test::More tests => 14;

# Create the rc file.
if (open my $fh, '>', 'add.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'add.rc', 'Created add.rc');
}

# Test the add command.
qx{../task rc:add.rc add This is a test};
my $output = qx{../task rc:add.rc info 1};
like ($output, qr/ID\s+1\n/, 'add ID');
like ($output, qr/Description\s+This is a test\n/, 'add ID');
like ($output, qr/Status\s+Pending\n/, 'add Pending');
like ($output, qr/UUID\s+[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}\n/, 'add UUID');

# Test the /// modifier.
qx{../task rc:add.rc 1 /test/TEST/};
qx{../task rc:add.rc 1 "/is //"};
$output = qx{../task rc:add.rc info 1};
like ($output, qr/ID\s+1\n/, 'add ID');
like ($output, qr/Status\s+Pending\n/, 'add Pending');
like ($output, qr/Description\s+This a TEST\n/, 'add ID');

# Test delete.
qx{../task rc:add.rc delete 1};
$output = qx{../task rc:add.rc info 1};
like ($output, qr/ID\s+1\n/, 'add ID');
like ($output, qr/Status\s+Deleted\n/, 'add Deleted');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'add.rc';
ok (!-r 'add.rc', 'Removed add.rc');

exit 0;

