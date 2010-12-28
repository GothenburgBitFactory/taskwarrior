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
if (open my $fh, '>', 'nag.rc')
{
  print $fh "data.location=.\n",
            "nag=NAG\n";
  close $fh;
  ok (-r 'nag.rc', 'Created nag.rc');
}

my $setup = "../src/task rc:nag.rc add due:yesterday one;"
          . "../src/task rc:nag.rc add due:tomorrow two;"
          . "../src/task rc:nag.rc add priority:H three;"
          . "../src/task rc:nag.rc add priority:M four;"
          . "../src/task rc:nag.rc add priority:L five;"
          . "../src/task rc:nag.rc add six;";
qx{$setup};

like   (qx{../src/task rc:nag.rc do 6}, qr/NAG/, 'do pri: -> nag');
like   (qx{../src/task rc:nag.rc do 5}, qr/NAG/, 'do pri:L -> nag');
like   (qx{../src/task rc:nag.rc do 4}, qr/NAG/, 'do pri:M-> nag');
like   (qx{../src/task rc:nag.rc do 3}, qr/NAG/, 'do pri:H-> nag');
like   (qx{../src/task rc:nag.rc do 2}, qr/NAG/, 'do due:tomorrow -> nag');
unlike (qx{../src/task rc:nag.rc do 1}, qr/NAG/, 'do due:yesterday -> no nag');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'nag.rc';
ok (!-r 'nag.rc', 'Removed nag.rc');

exit 0;

