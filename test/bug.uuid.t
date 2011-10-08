#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
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

