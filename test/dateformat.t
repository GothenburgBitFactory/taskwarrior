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
use Test::More tests => 15;

# Create the rc file.
if (open my $fh, '>', 'date1.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n",
            "dateformat.report=YMD\n";
  close $fh;
  ok (-r 'date1.rc', 'Created date1.rc');
}

if (open my $fh, '>', 'date2.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/y\n",
            "dateformat.report=m/d/y\n";
  close $fh;
  ok (-r 'date2.rc', 'Created date2.rc');
}

if (open my $fh, '>', 'date3.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/y\n",
            "dateformat=m/d/y\n",
            "weekstart=Monday\n",
            "dateformat.report=A D B Y (vV)\n";
  close $fh;
  ok (-r 'date3.rc', 'Created date3.rc');
}

qx{../src/task rc:date1.rc add foo due:20091231};
my $output = qx{../src/task rc:date1.rc info 1};
like ($output, qr/\b20091231\b/, 'date format YMD parsed');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

qx{../src/task rc:date2.rc add foo due:12/1/09};
$output = qx{../src/task rc:date2.rc info 1};
like ($output, qr/\b12\/1\/09\b/, 'date format m/d/y parsed');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

qx{../src/task rc:date3.rc add foo due:4/8/10};
$output = qx{../src/task rc:date3.rc list};
like ($output, qr/Thursday 08 April 2010 \(v14\)/, 'date format A D B Y (vV) parsed');
$output = qx{../src/task rc:date3.rc rc.dateformat.report:"D b Y - a" list};
like ($output, qr/08 Apr 2010 - Thu/, 'date format D b Y - a parsed');
                  
# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'date1.rc';
ok (!-r 'date1.rc', 'Removed date1.rc');

unlink 'date2.rc';
ok (!-r 'date2.rc', 'Removed date2.rc');

unlink 'date3.rc';
ok (!-r 'date3.rc', 'Removed date3.rc');

exit 0;

