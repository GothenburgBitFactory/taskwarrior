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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', 'due.rc')
{
  print $fh "data.location=.\n",
            "due=4\n",
            "color=on\n",
            "color.due=red\n",
            "color.alternate=\n",
            "_forcecolor=on\n",
            "dateformat=m/d/Y\n";
  close $fh;
  ok (-r 'due.rc', 'Created due.rc');
}

# Add a task that is almost due, and one that is just due.
my ($d, $m, $y) = (localtime (time + 3 * 86_400))[3..5];
my $just = sprintf ("%d/%d/%d", $m + 1, $d, $y + 1900);

($d, $m, $y) = (localtime (time + 5 * 86_400))[3..5];
my $almost = sprintf ("%d/%d/%d", $m + 1, $d, $y + 1900);

qx{../task rc:due.rc add one due:$just};
qx{../task rc:due.rc add two due:$almost};
my $output = qx{../task rc:due.rc list};
like ($output, qr/\[31m.+$just.+\[0m/, 'one marked due');
like ($output, qr/\s+$almost\s+/, 'two not marked due');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'due.rc';
ok (!-r 'due.rc', 'Removed due.rc');

exit 0;

