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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug #414: Tags filtering not working with unicode characters

# Add a task with a UTF-8 tag.
qx{../task rc:bug.rc add one +osobní};
my $output = qx{../task rc:bug.rc ls +osobní};
like ($output, qr/one/, 'found UTF8 tag osobní');

$output = qx{../task rc:bug.rc ls -osobní};
unlike ($output, qr/one/, 'not found UTF8 tag osobní');

# And a different one
qx{../task rc:bug.rc add two +föo};
$output = qx{../task rc:bug.rc ls +föo};
like ($output, qr/two/, 'found UTF8 tag föo');

$output = qx{../task rc:bug.rc ls -föo};
unlike ($output, qr/two/, 'not found UTF8 tag föo');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bug.rc';
ok (!-r 'bug.rc', 'Removed bug.rc');

exit 0;

