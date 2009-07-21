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
use Test::More tests => 12;

# Create the rc file.
if (open my $fh, '>', 'enp.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'enp.rc', 'Created enp.rc');
}

# Test the en passant feature.
qx{../task rc:enp.rc add foo};
qx{../task rc:enp.rc add foo bar};
qx{../task rc:enp.rc do 1,2 /foo/FOO/ pri:H +tag};
my $output = qx{../task rc:enp.rc info 1};
like ($output, qr/Status\s+Completed/,    'en passant 1 status change');
like ($output, qr/Description\s+FOO/,     'en passant 1 description change');
like ($output, qr/Priority\s+H/,          'en passant 1 description change');
like ($output, qr/Tags\s+tag/,            'en passant 1 description change');
$output = qx{../task rc:enp.rc info 2};
like ($output, qr/Status\s+Completed/,    'en passant 2 status change');
like ($output, qr/Description\s+FOO bar/, 'en passant 2 description change');
like ($output, qr/Priority\s+H/,          'en passant 2 description change');
like ($output, qr/Tags\s+tag/,            'en passant 2 description change');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'enp.rc';
ok (!-r 'enp.rc', 'Removed enp.rc');

exit 0;

