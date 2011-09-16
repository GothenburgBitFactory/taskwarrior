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
use Test::More tests => 3;

# Create the rc file.
if (open my $fh, '>', 'dom.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n";
  close $fh;
  ok (-r 'dom.rc', 'Created dom.rc');
}

qx{../src/task rc:dom.rc add one due:20110901};
qx{../src/task rc:dom.rc add two due:1.due};
my $output = qx{../src/task rc:dom.rc 2 info};
like ($output, qr/Due\s+20110901/, 'Found due date duplicated via dom');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key dom.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'dom.rc', 'Cleanup');

exit 0;

