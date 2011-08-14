#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
if (open my $fh, '>', 'custom.rc')
{
  print $fh "data.location=.\n",
            "report.foo.description=DESC\n",
            "report.foo.columns=id,recur.indicator\n",
            "report.foo.labels=ID,R\n",
            "report.foo.sort=id+\n";
  close $fh;
  ok (-r 'custom.rc', 'Created custom.rc');
}

# Add a recurring and non-recurring task, look for the indicator.
qx{../src/task rc:custom.rc add foo due:tomorrow recur:weekly};
qx{../src/task rc:custom.rc add bar};
my $output = qx{../src/task rc:custom.rc foo 2>&1};
like ($output,   qr/ID.+R/, 'Recurrence indicator heading');
like ($output,   qr/3\s+R/, 'Recurrence indicator t1');
unlike ($output, qr/2\s+R/, 'No recurrence indicator t2');

$output = qx{../src/task rc:custom.rc foo rc.recurrence.indicator=RE 2>&1};
like ($output,   qr/3\s+RE/, 'Custom recurrence indicator t1');
unlike ($output, qr/2\s+RE/, 'No custom recurrence indicator t2');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink 'custom.rc';
ok (!-r 'custom.rc', 'Removed custom.rc');

exit 0;

