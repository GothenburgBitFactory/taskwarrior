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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'uuid.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'uuid.rc', 'Created uuid.rc');
}

# Add a task, dup it, add a recurring task, list.  Then make sure they all have
# unique UUID values.
qx{../task rc:uuid.rc add simple};
qx{../task rc:uuid.rc 1 duplicate};
qx{../task rc:uuid.rc add periodic recur:daily due:yesterday};
my $output = qx{../task rc:uuid.rc stats};

my @all_uuids;
my %unique_uuids;
$output = qx{../task rc:uuid.rc 1 info};
my ($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../task rc:uuid.rc 2 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../task rc:uuid.rc 3 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../task rc:uuid.rc 4 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../task rc:uuid.rc 5 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../task rc:uuid.rc 6 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

is (scalar (@all_uuids), 6, '6 tasks created');
is (scalar (keys %unique_uuids), 6, '6 unique UUIDs');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'uuid.rc';
ok (!-r 'uuid.rc', 'Removed uuid.rc');

exit 0;

