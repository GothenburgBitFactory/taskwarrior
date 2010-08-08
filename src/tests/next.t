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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'next.rc')
{
  print $fh "data.location=.\n",
            "next=1\n";
  close $fh;
  ok (-r 'next.rc', 'Created next.rc');
}

# Add two tasks for each of two projects, then run next.  There should be only
# one task from each project shown.
qx{../task rc:next.rc add project:A priority:H AH};
qx{../task rc:next.rc add project:A priority:M AM};
qx{../task rc:next.rc add project:B priority:H BH};
qx{../task rc:next.rc add project:B Bnone};

my $output = qx{../task rc:next.rc next};
like ($output, qr/\s1\sA\s+H\s+(?:-|\d secs?)\sAH\n/, 'AH shown');
like ($output, qr/\s3\sB\s+H\s+(?:-|\d secs?)\sBH\n/, 'BH shown');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'next.rc';
ok (!-r 'next.rc', 'Removed next.rc');

exit 0;

