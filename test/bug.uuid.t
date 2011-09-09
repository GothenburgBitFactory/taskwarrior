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
use Test::More tests => 4;

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
qx{../src/task rc:uuid.rc add simple};
qx{../src/task rc:uuid.rc 1 duplicate};
qx{../src/task rc:uuid.rc add periodic recur:daily due:yesterday};
qx{../src/task rc:uuid.rc ls};

my @all_uuids;
my %unique_uuids;
my $output = qx{../src/task rc:uuid.rc 1 info};
my ($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../src/task rc:uuid.rc 2 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../src/task rc:uuid.rc 3 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../src/task rc:uuid.rc 4 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../src/task rc:uuid.rc 5 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

$output = qx{../src/task rc:uuid.rc 6 info};
($uuid) = $output =~ /UUID\s+(\S+)/;
push @all_uuids, $uuid;
$unique_uuids{$uuid} = undef;

is (scalar (@all_uuids), 6, '6 tasks created');
is (scalar (keys %unique_uuids), 6, '6 unique UUIDs');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key uuid.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'uuid.rc', 'Cleanup');

exit 0;

