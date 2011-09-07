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
            "confirmation=no\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test the -- argument.
qx{../src/task rc:args.rc add project:p pri:H +tag foo};
my $output = qx{../src/task rc:args.rc info 1};
like ($output, qr/Description\s+foo\n/ms, 'task add project:p pri:H +tag foo');

qx{../src/task rc:args.rc 1 modify project:p pri:H +tag -- foo};
$output = qx{../src/task rc:args.rc info 1};
like ($output, qr/Description\s+foo\n/ms, 'task 1 modify project:p pri:H +tag -- foo');

qx{../src/task rc:args.rc 1 modify project:p pri:H -- +tag foo};
$output = qx{../src/task rc:args.rc info 1};
like ($output, qr/Description\s+\+tag\sfoo\n/ms, 'task 1 modify project:p pri:H -- +tag foo');

qx{../src/task rc:args.rc 1 modify project:p -- pri:H +tag foo};
$output = qx{../src/task rc:args.rc info 1};
like ($output, qr/Description\s+pri:H\s\+tag\sfoo\n/ms, 'task 1 modify project:p -- pri:H +tag foo');

qx{../src/task rc:args.rc 1 modify -- project:p pri:H +tag foo};
$output = qx{../src/task rc:args.rc info 1};
like ($output, qr/Description\s+project:p\spri:H\s\+tag\sfoo\n/ms, 'task 1 modify -- project:p pri:H +tag foo');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key args.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'args.rc', 'Cleanup');

exit 0;

