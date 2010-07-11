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
if (open my $fh, '>', 'or.rc')
{
  print $fh "data.location=.\n",
            "annotations=none\n",
            "report.zzz.columns=id,due,description\n",
            "report.zzz.labels=ID,Due,Description\n",
            "report.zzz.sort=due+\n",
            "report.zzz.filter=status:pending rc.annotations:full\n";
  close $fh;
  ok (-r 'or.rc', 'Created or.rc');
}

# The zzz report is defined with an override in the filter that contradicts
# the value in the rc.  The filter override should prevail.
qx{../task rc:or.rc add ONE};
qx{../task rc:or.rc 1 annotate TWO};
my $output = qx{../task rc:or.rc zzz};
like ($output, qr/ONE.+TWO/ms, 'filter override > rc setting');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'or.rc';
ok (!-r 'or.rc', 'Removed or.rc');

exit 0;

