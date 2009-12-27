#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'import.rc', 'Created import.rc');
}

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh "'id','uuid','status','tags','entry','start','due','recur','end','project','priority','fg','bg','description'\n",
            "'7f7a4191-c2f2-487f-8855-7a1eb378c267','pending','',1238037947,,,,,'A','M',,,'foo bar'\n",
            "'7f7a4191-c2f2-487f-8855-7a1eb378c267','pending','',1238037947,,,,,'A','M',,,'foo, bar'\n",
            "\n";
  close $fh;
  ok (-r 'import.txt', 'Created sample import data');
}

my $output = qx{../task rc:import.rc import import.txt};
like ($output, qr/Imported 2 tasks successfully, with 0 errors./, 'no errors');

$output = qx{../task rc:import.rc list};
like ($output, qr/1.+A.+M.+foo bar/,  't1');
like ($output, qr/2.+A.+M.+foo, bar/, 't2');

# Cleanup.
unlink 'import.txt';
ok (!-r 'import.txt', 'Removed import.txt');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'import.rc';
ok (!-r 'import.rc', 'Removed import.rc');

exit 0;

