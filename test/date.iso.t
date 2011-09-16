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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'iso.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/Y\n";
  close $fh;
  ok (-r 'iso.rc', 'Created iso.rc');
}

# Test use of ISO date format, despite rc.dateformat.
qx{../src/task rc:iso.rc add one due:20110901T120000Z};
my $output = qx{../src/task rc:iso.rc 1 info};
like ($output, qr/Due\s+9\/1\/2011/, 'ISO format recognized.');

# Test use of epoch date format, despite rc.dateformat.
qx{../src/task rc:iso.rc add one due:1234567890};
$output = qx{../src/task rc:iso.rc 2 info};
like ($output, qr/Due\s+2\/13\/2009/, 'Epoch format recognized.');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key iso.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'iso.rc', 'Cleanup');

exit 0;

