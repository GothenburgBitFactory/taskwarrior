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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test 'stop' with en-passant changes.
qx{../src/task rc:args.rc add one};
qx{../src/task rc:args.rc add two};
qx{../src/task rc:args.rc add three};
qx{../src/task rc:args.rc add four};
qx{../src/task rc:args.rc add five};

qx{../src/task rc:args.rc 1 start};
qx{../src/task rc:args.rc 2 start};
qx{../src/task rc:args.rc 3 start};
qx{../src/task rc:args.rc 4 start};
qx{../src/task rc:args.rc 5 start};

qx{../src/task rc:args.rc 1 stop oneanno};
my $output = qx{../src/task rc:args.rc 1 info};
like ($output, qr/oneanno/, 'stop enpassant anno');

qx{../src/task rc:args.rc 2 stop /two/TWO/};
$output = qx{../src/task rc:args.rc 2 info};
like ($output, qr/Description\s+TWO/, 'stop enpassant subst');

qx{../src/task rc:args.rc 3 stop +threetag};
$output = qx{../src/task rc:args.rc 3 info};
like ($output, qr/Tags\s+threetag/, 'stop enpassant tag');

qx{../src/task rc:args.rc 4 stop pri:H};
$output = qx{../src/task rc:args.rc 4 info};
like ($output, qr/Priority\s+H/, 'stop enpassant priority');

qx{../src/task rc:args.rc 5 stop pro:A};
$output = qx{../src/task rc:args.rc 5 info};
like ($output, qr/Project\s+A/, 'stop enpassant project');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key args.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'args.rc', 'Cleanup');

exit 0;

