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
if (open my $fh, '>', 'range.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'range.rc', 'Created range.rc');
}

# Add three tasks, and attempt to list the middle one within a range.
qx{../src/task rc:range.rc add one due:8/1/2009};
qx{../src/task rc:range.rc add two due:8/3/2009};
qx{../src/task rc:range.rc add three due:8/5/2009};
my $output = qx{../src/task rc:range.rc ls due.after:8/2/2009 due.before:8/4/2009};
unlike ($output, qr/one/,   'Missing prior to range');
like   ($output, qr/two/,   'Found within range');
unlike ($output, qr/three/, 'Missing after range');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'range.rc';
ok (!-r 'range.rc', 'Removed range.rc');

exit 0;

