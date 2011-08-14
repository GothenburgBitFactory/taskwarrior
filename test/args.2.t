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
use Test::More tests => 12;

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test 'done' with en-passant changes.
qx{../src/task rc:args.rc add one};
qx{../src/task rc:args.rc add two};
qx{../src/task rc:args.rc add three};
qx{../src/task rc:args.rc add four};
qx{../src/task rc:args.rc add five};

qx{../src/task rc:args.rc 1 done oneanno};
my $output = qx{../src/task rc:args.rc 1 info};
like ($output, qr/oneanno/, 'done enpassant anno');

qx{../src/task rc:args.rc 2 done /two/TWO/};
$output = qx{../src/task rc:args.rc 2 info};
like ($output, qr/Description\s+TWO/, 'done enpassant subst');

qx{../src/task rc:args.rc 3 done +threetag};
$output = qx{../src/task rc:args.rc 3 info};
like ($output, qr/Tags\s+threetag/, 'done enpassant tag');

qx{../src/task rc:args.rc 4 done pri:H};
$output = qx{../src/task rc:args.rc 4 info};
like ($output, qr/Priority\s+H/, 'done enpassant priority');

qx{../src/task rc:args.rc 5 done pro:A};
$output = qx{../src/task rc:args.rc 5 info};
like ($output, qr/Project\s+A/, 'done enpassant project');

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

unlink 'args.rc';
ok (!-r 'args.rc', 'Removed args.rc');

exit 0;

