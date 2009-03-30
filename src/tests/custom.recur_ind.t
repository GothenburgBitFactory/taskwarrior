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
if (open my $fh, '>', 'custom.rc')
{
  print $fh "data.location=.\n",
            "report.foo.description=DESC\n",
            "report.foo.columns=id,recurrence_indicator\n",
            "report.foo.labels=ID,R\n",
            "report.foo.sort=id+\n";
  close $fh;
  ok (-r 'custom.rc', 'Created custom.rc');
}

# Generate the usage screen, and locate the custom report on it.
qx{../task rc:custom.rc add foo due:tomorrow recur:weekly};
qx{../task rc:custom.rc add bar};
my $output = qx{../task rc:custom.rc foo 2>&1};
like ($output,   qr/ID R/,   'Recurrence indicator heading');
like ($output,   qr/3\s+R/, 'Recurrence indicator t1');
unlike ($output, qr/2\s+R/, 'No recurrence indicator t2');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'custom.rc';
ok (!-r 'custom.rc', 'Removed custom.rc');

exit 0;

