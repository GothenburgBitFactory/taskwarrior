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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  print $fh "bulk=100\n";
  print $fh "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 804: URL link and break line

# Setup: Add a tasks, annotate with long word.
qx{../src/task rc:bug.rc add One};
qx{../src/task rc:bug.rc 1 annotate abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz};

# List with rc.hyphenate=on.
my $output = qx{../src/task rc:bug.rc rc.defaultwidth:40 rc.hyphenate:on ls};
like ($output, qr/vwx-$/ms, 'hyphenated 1');
like ($output, qr/tuv-$/ms, 'hyphenated 2');

# List with rc.hyphenate=off.
$output = qx{../src/task rc:bug.rc rc.defaultwidth:40 rc.hyphenate:off ls};
like ($output, qr/vwxy$/ms, 'not hyphenated 1');
like ($output, qr/uvwx$/ms, 'not hyphenated 2');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

