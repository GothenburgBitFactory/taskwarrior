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
use Test::More tests => 27;

# Create the rc file.
if (open my $fh, '>', 'start.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'start.rc', 'Created start.rc');
}

# Test the add/start/stop commands.
qx{../src/task rc:start.rc add one};
qx{../src/task rc:start.rc add two};
my $output = qx{../src/task rc:start.rc active};
unlike ($output, qr/one/, 'one not active');
unlike ($output, qr/two/, 'two not active');

qx{../src/task rc:start.rc 1 start};
qx{../src/task rc:start.rc 2 start};
$output = qx{../src/task rc:start.rc active};
like ($output, qr/one/, 'one active');
like ($output, qr/two/, 'two active');

qx{../src/task rc:start.rc 1 stop};
$output = qx{../src/task rc:start.rc active};
unlike ($output, qr/one/, 'one not active');
like   ($output, qr/two/, 'two active');

qx{../src/task rc:start.rc 2 stop};
$output = qx{../src/task rc:start.rc active};
unlike ($output, qr/one/, 'one not active');
unlike ($output, qr/two/, 'two not active');

qx{../src/task rc:start.rc 2 done};
$output = qx{../src/task rc:start.rc list};
unlike ($output, qr/two/, 'two deleted');

# Create the rc file.
if (open my $fh, '>', 'start2.rc')
{
  print $fh "data.location=.\n",
            "journal.time=on\n";
  close $fh;
  ok (-r 'start2.rc', 'Created start2.rc');
}

qx{../src/task rc:start2.rc 1 start};
$output = qx{../src/task rc:start2.rc list};
like ($output, qr/Started task/, 'one start and annotated');

qx{../src/task rc:start2.rc 1 stop};
$output = qx{../src/task rc:start2.rc list};
like ($output, qr/Stopped task/, 'one stopped and annotated');

# Create the rc file.
if (open my $fh, '>', 'start3.rc')
{
  print $fh "data.location=.\n",
            "journal.time=on\n",
            "journal.time.start.annotation=Nu kör vi\n",
            "journal.time.stop.annotation=Nu stannar vi\n";
  close $fh;
  ok (-r 'start3.rc', 'Created start3.rc');
}

qx{../src/task rc:start3.rc 1 start};
$output = qx{../src/task rc:start3.rc list};
like ($output, qr/Nu.+kör.+vi/ms, 'one start and annotated with custom description');

qx{../src/task rc:start3.rc 1 stop};
$output = qx{../src/task rc:start3.rc list};
like ($output, qr/Nu.+stannar.+vi/ms, 'one stopped and annotated with custom description');

# Cleanup.
ok (-r 'pending.data', 'Need to remove pending.data');
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

ok (-r 'completed.data', 'Need to remove completed.data');
unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

ok (-r 'undo.data', 'Need to remove undo.data');
unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink 'start.rc';
ok (!-r 'start.rc', 'Removed start.rc');
unlink 'start2.rc';
ok (!-r 'start2.rc', 'Removed start2.rc');
unlink 'start3.rc';
ok (!-r 'start3.rc', 'Removed start3.rc');

exit 0;

