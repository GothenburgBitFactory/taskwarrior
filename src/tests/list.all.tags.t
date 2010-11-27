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
use Test::More tests => 11;

# Create the rc file.
if (open my $fh, '>', 'tags.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'tags.rc', 'Created tags.rc');
}

# Create a data set of two tasks, with unique project names, one
# pending, one completed.
qx{../task rc:tags.rc add +t1 one};
qx{../task rc:tags.rc add +t2 two};
qx{../task rc:tags.rc done 1};
my $output = qx{../task rc:tags.rc long};
unlike ($output, qr/t1/, 't1 done');
like ($output, qr/t2/, 't2 pending');

$output = qx{../task rc:tags.rc tags};
unlike ($output, qr/t1/, 't1 done');
like ($output, qr/t2/, 't2 pending');

$output = qx{../task rc:tags.rc rc.list.all.tags:yes tags};
like ($output, qr/t1/, 't1 listed');
like ($output, qr/t2/, 't2 listed');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'tags.rc';
ok (!-r 'tags.rc', 'Removed tags.rc');

exit 0;

