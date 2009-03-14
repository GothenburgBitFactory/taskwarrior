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
if (open my $fh, '>', 'date1.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n";
  close $fh;
  ok (-r 'date1.rc', 'Created date1.rc');
}

if (open my $fh, '>', 'date2.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/y\n";
  close $fh;
  ok (-r 'date2.rc', 'Created date2.rc');
}

qx{../task rc:date1.rc add foo due:20091231};
my $output = qx{../task rc:date1.rc info 1};
like ($output, qr/\b20091231\b/, 'date format YMD parsed');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

qx{../task rc:date2.rc add foo due:12/1/09};
$output = qx{../task rc:date2.rc info 1};
like ($output, qr/\b12\/1\/09\b/, 'date format m/d/y parsed');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'date1.rc';
ok (!-r 'date1.rc', 'Removed date1.rc');

unlink 'date2.rc';
ok (!-r 'date2.rc', 'Removed date2.rc');

exit 0;

