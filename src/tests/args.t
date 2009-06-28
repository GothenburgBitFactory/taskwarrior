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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test the -- argument.
qx{../task rc:args.rc add project:p pri:H +tag foo};
my $output = qx{../task rc:args.rc info 1};
like ($output, qr/Description\s+foo\n/ms, 'task add project:p pri:H +tag foo');

qx{../task rc:args.rc 1 project:p pri:H +tag -- foo};
$output = qx{../task rc:args.rc info 1};
like ($output, qr/Description\s+foo\n/ms, 'task 1 project:p pri:H +tag -- foo');

qx{../task rc:args.rc 1 project:p pri:H -- +tag foo};
$output = qx{../task rc:args.rc info 1};
like ($output, qr/Description\s+\+tag\sfoo\n/ms, 'task 1 project:p pri:H -- +tag foo');

qx{../task rc:args.rc 1 project:p -- pri:H +tag foo};
$output = qx{../task rc:args.rc info 1};
like ($output, qr/Description\s+pri:H\s\+tag\sfoo\n/ms, 'task 1 project:p -- pri:H +tag foo');

qx{../task rc:args.rc 1 -- project:p pri:H +tag foo};
$output = qx{../task rc:args.rc info 1};
like ($output, qr/Description\s+project:p\spri:H\s\+tag\sfoo\n/ms, 'task 1 -- project:p pri:H +tag foo');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'args.rc';
ok (!-r 'args.rc', 'Removed args.rc');

exit 0;

