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
use Test::More tests => 39;

# Create the rc file.
if (open my $fh, '>', 'pri.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'pri.rc', 'Created pri.rc');
}

# Verify that priorities can be select with the 'over' and 'under' modifiers.
qx{../src/task rc:pri.rc add H pri:H};
qx{../src/task rc:pri.rc add M pri:M};
qx{../src/task rc:pri.rc add L pri:L};
qx{../src/task rc:pri.rc add _};

my $output = qx{../src/task rc:pri.rc ls priority.under:H};
unlike ($output, qr/H/, 'pri H !< H');
  like ($output, qr/M/, 'pri M < H');
  like ($output, qr/L/, 'pri L < H');
  like ($output, qr/_/, 'pri _ < H');

$output = qx{../src/task rc:pri.rc ls priority.under:M};
unlike ($output, qr/H/, 'pri H !< M');
unlike ($output, qr/M/, 'pri M !< M');
  like ($output, qr/L/, 'pri L < M');
  like ($output, qr/_/, 'pri _ < M');

$output = qx{../src/task rc:pri.rc ls priority.under:L};
unlike ($output, qr/H/, 'pri H !< L');
unlike ($output, qr/M/, 'pri M !< L');
unlike ($output, qr/L/, 'pri L !< L');
  like ($output, qr/_/, 'pri _ < L');

$output = qx{../src/task rc:pri.rc ls priority.under:};
unlike ($output, qr/H/, 'pri H !< _');
unlike ($output, qr/M/, 'pri M !< _');
unlike ($output, qr/L/, 'pri L !< _');
unlike ($output, qr/_/, 'pri _ !< _');

$output = qx{../src/task rc:pri.rc ls priority.over:H};
unlike ($output, qr/H/, 'pri H !> H');
unlike ($output, qr/M/, 'pri M !> H');
unlike ($output, qr/L/, 'pri L !> H');
unlike ($output, qr/_/, 'pri _ !> H');

$output = qx{../src/task rc:pri.rc ls priority.over:M};
  like ($output, qr/H/, 'pri H > M');
unlike ($output, qr/M/, 'pri M !> M');
unlike ($output, qr/L/, 'pri L !> M');
unlike ($output, qr/_/, 'pri _ !> M');

$output = qx{../src/task rc:pri.rc ls priority.over:L};
  like ($output, qr/H/, 'pri H > L');
  like ($output, qr/M/, 'pri M > L');
unlike ($output, qr/L/, 'pri L !> L');
unlike ($output, qr/_/, 'pri _ !> L');

$output = qx{../src/task rc:pri.rc ls priority.over:};
  like ($output, qr/H/, 'pri H > _');
  like ($output, qr/M/, 'pri M > _');
  like ($output, qr/L/, 'pri L > _');
unlike ($output, qr/_/, 'pri _ !> _');

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

unlink 'pri.rc';
ok (!-r 'pri.rc', 'Removed pri.rc');

exit 0;

