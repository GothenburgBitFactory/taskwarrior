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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'uuid.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'uuid.rc', 'Created uuid.rc');
}

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[description:"one" entry:"1315260230" status:"pending" uuid:"9deed7ca-843d-4259-b2c4-40ce73e8e4f3"]
[description:"two" entry:"1315260230" status:"pending" uuid:"0f4c83d2-552f-4108-ae3f-ccc7959f84a3"]
[description:"three" entry:"1315260230" status:"pending" uuid:"aa4abef1-1dc5-4a43-b6a0-7872df3094bb"]
EOF
  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

if (open my $fh, '>', 'completed.data')
{
  print $fh <<EOF;
[description:"four" end:"1315260230" entry:"1315260230" status:"completed" uuid:"ea3b4822-574c-464b-8025-7f7be9f3cc57"]
[description:"five" end:"1315260230" entry:"1315260230" status:"completed" uuid:"0f38b97e-3081-4e75-a1be-65ed3712ea4d"]
EOF
  close $fh;
  ok (-r 'pending.data', 'Created completed.data');
}

qx{../src/task rc:uuid.rc 9deed7ca-843d-4259-b2c4-40ce73e8e4f3 modify ONE};
qx{../src/task rc:uuid.rc 2 modify TWO};
my $output = qx{../src/task rc:uuid.rc list};
like ($output, qr/ONE/, 'task ONE');
like ($output, qr/TWO/, 'task TWO');
like ($output, qr/three/, 'task three');

qx{../src/task rc:uuid.rc ea3b4822-574c-464b-8025-7f7be9f3cc57 modify FOUR};
$output = qx{../src/task rc:uuid.rc all};
like ($output, qr/FOUR/, 'task FOUR');
like ($output, qr/five/, 'task five');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key uuid.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'uuid.rc', 'Cleanup');

exit 0;

