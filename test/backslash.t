#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 3;

# Create the rc file.
if (open my $fh, '>', 'backslash.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'backslash.rc', 'Created backslash.rc');
}

# Add a description with a backslash.
qx{../src/task rc:backslash.rc add foo\\\\bar};
my $output = qx{../src/task rc:backslash.rc ls};
like ($output, qr/foo\\bar/, 'Backslash preserved, no parsing issues');

# Cleanup.
unlink 'backslash.rc';
ok (!-r 'backslash.rc', 'Removed backslash.rc');

exit 0;

################################################################################
