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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'summary.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'summary.rc', 'Created summary.rc');
}

# Add three tasks.  Do 1, delete 1, leave 1 pending.  Summary should depict a
# 50% completion.
qx{../task rc:summary.rc add project:A one};
qx{../task rc:summary.rc add project:A two};
qx{../task rc:summary.rc add project:A three};
qx{../task rc:summary.rc do 1};
qx{../task rc:summary.rc delete 2};
my $output = qx{../task rc:summary.rc summary};
like ($output, qr/A\s+1\s+-\s+50%/, 'summary correctly shows 50% before report');

qx{../task rc:summary.rc list};
$output = qx{../task rc:summary.rc summary};
like ($output, qr/A\s+1\s+-\s+50%/, 'summary correctly shows 50% after report');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'summary.rc';
ok (!-r 'summary.rc', 'Removed summary.rc');

exit 0;

