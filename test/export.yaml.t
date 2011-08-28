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
use Test::More tests => 20;

# Create the rc file.
if (open my $fh, '>', 'export.rc')
{
  print $fh "data.location=.\n",
            "verbose=no\n";
  close $fh;
  ok (-r 'export.rc', 'Created export.rc');
}

# Add two tasks, export, examine result.
qx{../src/task rc:export.rc add priority:H project:A one};
qx{../src/task rc:export.rc add +tag1 +tag2 two};
diag (qx{env PATH=/usr/bin:../src ../scripts/add-ons/export-yaml.pl rc:export.rc > ./export.txt});

my @lines;
if (open my $fh, '<', './export.txt')
{
  @lines = <$fh>;
  close $fh;
}

like ($lines[0],  qr/^\%YAML 1\.1$/,             'export YAML line 1');
like ($lines[1],  qr/^---$/,                     'export YAML line 2');
like ($lines[2],  qr/^  task:$/,                 'export YAML line 3');
like ($lines[3],  qr/^    description: one$/,    'export YAML line 4');
like ($lines[4],  qr/^    entry: \d{8}T\d{6}Z$/, 'export YAML line 5');
like ($lines[5],  qr/^    id: \d+$/,             'export YAML line 6');
like ($lines[6],  qr/^    priority: H$/,         'export YAML line 7');
like ($lines[7],  qr/^    project: A$/,          'export YAML line 8');
like ($lines[8],  qr/^    status: pending$/,     'export YAML line 9');
like ($lines[9],  qr/^    uuid: .+$/,            'export YAML line 10');
like ($lines[10], qr/^  task:$/,                 'export YAML line 11');
like ($lines[11], qr/^    description: two$/,    'export YAML line 12');
like ($lines[12], qr/^    entry: \d{8}T\d{6}Z$/, 'export YAML line 13');
like ($lines[13], qr/^    id: \d+$/,             'export YAML line 14');
like ($lines[14], qr/^    status: pending$/,     'export YAML line 15');
like ($lines[15], qr/^    tags: tag1,tag2$/,     'export YAML line 16');
like ($lines[16], qr/^    uuid: .+$/,            'export YAML line 17');
like ($lines[17], qr/^\.\.\.$/,                  'export YAML line 18');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key export.rc export.txt);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'export.rc'      &&
    ! -r 'export.txt', 'Cleanup');

exit 0;

