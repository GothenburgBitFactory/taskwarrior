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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', 'duplicate.rc')
{
  print $fh "data.location=.\n",
            "data.location=.\n",
            "color=off\n";
  close $fh;
  ok (-r 'duplicate.rc', 'Created duplicate.rc');
}

# Test the add command.
my $output = qx{../task rc:duplicate.rc rc.longversion:off version};
like   ($output, qr/data\.location/ms,  'Duplicate entry detected');
unlike ($output, qr/colorl/ms, 'Single entry not ignored');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'duplicate.rc';
ok (!-r 'duplicate.rc', 'Removed duplicate.rc');

exit 0;

