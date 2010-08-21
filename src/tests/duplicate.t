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
use Test::More tests => 16;

# Create the rc file.
if (open my $fh, '>', 'dup.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'dup.rc', 'Created dup.rc');
}

# Test the duplicate command.
qx{../task rc:dup.rc add foo};
qx{../task rc:dup.rc duplicate 1};
my $output = qx{../task rc:dup.rc info 2};
like ($output, qr/ID\s+2/,            'duplicate new id');
like ($output, qr/Status\s+Pending/,  'duplicate same status');
like ($output, qr/Description\s+foo/, 'duplicate same description');

# Test the en passant modification while duplicating.
qx{../task rc:dup.rc duplicate 1 priority:H /foo/FOO/ +tag};
$output = qx{../task rc:dup.rc info 3};
like ($output, qr/ID\s+3/,            'duplicate new id');
like ($output, qr/Status\s+Pending/,  'duplicate same status');
like ($output, qr/Description\s+FOO/, 'duplicate modified description');
like ($output, qr/Priority\s+H/,      'duplicate added priority');
like ($output, qr/Tags\s+tag/,        'duplicate added tag');

# Test the output of the duplicate command - returning id of duplicated task
$output = qx{../task rc:dup.rc duplicate 1};
like ($output, qr/Duplicated\s+1\s+'foo'/, 'duplicate output task id and description');
like ($output, qr/Duplicated\s+1\s+task/,  'duplicate output number of tasks duplicated');
like ($output, qr/Created\s+task\s+4/,     'duplicate output of new task id');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'dup.rc';
ok (!-r 'dup.rc', 'Removed dup.rc');

exit 0;

