#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'append.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'append.rc', 'Created append.rc');
}

# Add a task, then append more decsription.
qx{../task rc:append.rc add foo};
qx{../task rc:append.rc 1 append bar};
my $output = qx{../task rc:append.rc info 1};
like ($output, qr/Description\s+foo\sbar\n/, 'append worked');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'append.rc';
ok (!-r 'append.rc', 'Removed append.rc');

exit 0;

