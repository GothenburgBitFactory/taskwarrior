#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
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
use Test::More tests => 13;

# Create the rc file.
if (open my $fh, '>', 'wait.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";

  close $fh;
  ok (-r 'wait.rc', 'Created wait.rc');
}

# Add a waiting task, check it is not there, wait, then check it is.
qx{../task rc:wait.rc add yeswait wait:2s};
qx{../task rc:wait.rc add nowait};

my $output = qx{../task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task visible');
unlike ($output, qr/yeswait/ms, 'waiting task invisible');

diag ("3 second delay");
sleep 3;

$output = qx{../task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task still visible');
like ($output, qr/yeswait/ms, 'waiting task now visible');

qx{../task rc:wait.rc 1 wait:2s};
$output = qx{../task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task visible');
unlike ($output, qr/yeswait/ms, 'waiting task invisible');

diag ("3 second delay");
sleep 3;

$output = qx{../task rc:wait.rc ls};
like ($output, qr/nowait/ms, 'non-waiting task still visible');
like ($output, qr/yeswait/ms, 'waiting task now visible');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'wait.rc';
ok (!-r 'wait.rc', 'Removed wait.rc');

exit 0;
