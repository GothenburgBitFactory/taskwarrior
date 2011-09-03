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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'projects.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'projects.rc', 'Created projects.rc');
}

# Create a data set of two tasks, with unique project names, one
# pending, one completed.
qx{../src/task rc:projects.rc add project:p1 one};
qx{../src/task rc:projects.rc add project:p2 two};
qx{../src/task rc:projects.rc 1 done};

my $output = qx{../src/task rc:projects.rc ls};
unlike ($output, qr/p1/, 'p1 done');
like ($output, qr/p2/, 'p2 pending');

$output = qx{../src/task rc:projects.rc projects};
unlike ($output, qr/p1/, 'p1 done');
like ($output, qr/p2/, 'p2 pending');

$output = qx{../src/task rc:projects.rc rc.list.all.projects:yes projects};
like ($output, qr/p1/, 'p1 listed');
like ($output, qr/p2/, 'p2 listed');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key projects.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'projects.rc', 'Cleanup');

exit 0;

