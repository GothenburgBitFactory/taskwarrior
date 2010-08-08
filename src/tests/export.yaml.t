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
use Test::More tests => 21;

# Create the rc file.
if (open my $fh, '>', 'export.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'export.rc', 'Created export.rc');
}

# Add two tasks, export, examine result.
qx{../task rc:export.rc add priority:H project:A one};
qx{../task rc:export.rc add +tag1 +tag2 two};
qx{../task rc:export.rc export.yaml > ./export.txt};

my @lines;
if (open my $fh, '<', './export.txt')
{
  @lines = <$fh>;
  close $fh;
}

like ($lines[0],  qr/^\%YAML 1\.1$/,          'export.yaml line 1');
like ($lines[1],  qr/^---$/,                  'export.yaml line 2');
like ($lines[2],  qr/^  task:$/,              'export.yaml line 3');
like ($lines[3],  qr/^    description: one$/, 'export.yaml line 4');
like ($lines[4],  qr/^    entry: \d+$/,       'export.yaml line 5');
like ($lines[5],  qr/^    priority: H$/,      'export.yaml line 6');
like ($lines[6],  qr/^    project: A$/,       'export.yaml line 7');
like ($lines[7],  qr/^    status: pending$/,  'export.yaml line 8');
like ($lines[8],  qr/^    uuid: .+$/,         'export.yaml line 9');
like ($lines[9],  qr/^  task:$/,              'export.yaml line 10');
like ($lines[10], qr/^    description: two$/, 'export.yaml line 11');
like ($lines[11], qr/^    entry: \d+$/,       'export.yaml line 12');
like ($lines[12], qr/^    status: pending$/,  'export.yaml line 13');
like ($lines[13], qr/^    tags: tag1,tag2$/,  'export.yaml line 14');
like ($lines[14], qr/^    uuid: .+$/,         'export.yaml line 15');
like ($lines[15], qr/^\.\.\.$/,               'export.yaml line 16');

# Cleanup.
unlink 'export.txt';
ok (!-r 'export.txt', 'Removed export.txt');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'export.rc';
ok (!-r 'export.rc', 'Removed export.rc');

exit 0;

