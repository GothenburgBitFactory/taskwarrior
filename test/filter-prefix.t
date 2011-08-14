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
use Test::More tests => 63;

# Create the rc file.
if (open my $fh, '>', 'filter.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'filter.rc', 'Created filter.rc');
}

# Test the filters.
qx{../src/task rc:filter.rc add project:foo.uno priority:H +tag one foo};
qx{../src/task rc:filter.rc add project:foo.dos priority:H      two};
qx{../src/task rc:filter.rc add project:foo.tres                three};
qx{../src/task rc:filter.rc add project:bar.uno priority:H      four};
qx{../src/task rc:filter.rc add project:bar.dos            +tag five};
qx{../src/task rc:filter.rc add project:bar.tres                six foo};
qx{../src/task rc:filter.rc add project:bazuno                  seven bar foo};
qx{../src/task rc:filter.rc add project:bazdos                  eight bar foo};

my $output = qx{../src/task rc:filter.rc list};
like   ($output, qr/one/,   'a1');
like   ($output, qr/two/,   'a2');
like   ($output, qr/three/, 'a3');
like   ($output, qr/four/,  'a4');
like   ($output, qr/five/,  'a5');
like   ($output, qr/six/,   'a6');
like   ($output, qr/seven/, 'a7');
like   ($output, qr/eight/, 'a8');

$output = qx{../src/task rc:filter.rc list project:foo};
like   ($output, qr/one/,   'b1');
like   ($output, qr/two/,   'b2');
like   ($output, qr/three/, 'b3');
unlike ($output, qr/four/,  'b4');
unlike ($output, qr/five/,  'b5');
unlike ($output, qr/six/,   'b6');
unlike ($output, qr/seven/, 'b7');
unlike ($output, qr/eight/, 'b8');

$output = qx{../src/task rc:filter.rc list project.not:foo};
unlike ($output, qr/one/,   'c1');
unlike ($output, qr/two/,   'c2');
unlike ($output, qr/three/, 'c3');
like   ($output, qr/four/,  'c4');
like   ($output, qr/five/,  'c5');
like   ($output, qr/six/,   'c6');
like   ($output, qr/seven/, 'c7');
like   ($output, qr/eight/, 'c8');

$output = qx{../src/task rc:filter.rc list project.startswith:bar};
unlike ($output, qr/one/,   'd1');
unlike ($output, qr/two/,   'd2');
unlike ($output, qr/three/, 'd3');
like   ($output, qr/four/,  'd4');
like   ($output, qr/five/,  'd5');
like   ($output, qr/six/,   'd6');
unlike ($output, qr/seven/, 'd7');
unlike ($output, qr/eight/, 'd8');

$output = qx{../src/task rc:filter.rc list project:ba};
unlike ($output, qr/one/,   'f1');
unlike ($output, qr/two/,   'f2');
unlike ($output, qr/three/, 'f3');
like   ($output, qr/four/,  'f4');
like   ($output, qr/five/,  'f5');
like   ($output, qr/six/,   'f6');
like   ($output, qr/seven/, 'f7');
like   ($output, qr/eight/, 'f8');

$output = qx{../src/task rc:filter.rc list project.not:ba};
like   ($output, qr/one/,   'g1');
like   ($output, qr/two/,   'g2');
like   ($output, qr/three/, 'g3');
unlike ($output, qr/four/,  'g4');
unlike ($output, qr/five/,  'g5');
unlike ($output, qr/six/,   'g6');
unlike ($output, qr/seven/, 'g7');
unlike ($output, qr/eight/, 'g8');

$output = qx{../src/task rc:filter.rc list description.has:foo};
like   ($output, qr/one/,   'i1');
unlike ($output, qr/two/,   'i2');
unlike ($output, qr/three/, 'i3');
unlike ($output, qr/four/,  'i4');
unlike ($output, qr/five/,  'i5');
like   ($output, qr/six/,   'i6');
like   ($output, qr/seven/, 'i7');
like   ($output, qr/eight/, 'i8');

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

unlink 'filter.rc';
ok (!-r 'filter.rc', 'Removed filter.rc');

exit 0;

