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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'bug_annotate.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug_annotate.rc', 'Created bug_annotate.rc');
}

# Attempt a blank annotation.
qx{../task rc:bug_annotate.rc add foo};
my $output = qx{../task rc:bug_annotate.rc 1 annotate};
like ($output, qr/Cannot apply a blank annotation./, 'failed on blank annotation');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'bug_annotate.rc';
ok (!-r 'bug_annotate.rc', 'Removed bug_annotate.rc');

exit 0;

