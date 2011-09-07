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
use Test::More tests => 26;

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
[description:"ssttaarrtt" entry:"1315335826" start:"1315338535" status:"pending" uuid:"d71d3566-7a6b-4c32-8f0b-6de75bb9397b"]
EOF
  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

if (open my $fh, '>', 'completed.data')
{
  print $fh <<EOF;
[description:"four" end:"1315260230" entry:"1315260230" status:"completed" uuid:"ea3b4822-574c-464b-8025-7f7be9f3cc57"]
[description:"five" end:"1315260230" entry:"1315260230" status:"completed" uuid:"0f38b97e-3081-4e75-a1be-65ed3712ea4d"]
[description:"eenndd" end:"1315335841" entry:"1315335841" start:"1315338516" status:"completed" uuid:"727baa6c-65b8-485e-a810-e133e3cd83dc"]
[description:"UUNNDDOO" end:"1315338626" entry:"1315338626" status:"completed" uuid:"c1361003-948e-43e8-85c8-15d28dc3c71c"]
EOF
  close $fh;
  ok (-r 'pending.data', 'Created completed.data');
}

qx{../src/task rc:uuid.rc 9deed7ca-843d-4259-b2c4-40ce73e8e4f3 modify ONE};
qx{../src/task rc:uuid.rc 2 modify TWO};
my $output = qx{../src/task rc:uuid.rc list};
like ($output, qr/ONE/, 'task list ONE');
like ($output, qr/TWO/, 'task list TWO');
like ($output, qr/three/, 'task list three');
like ($output, qr/ssttaarrtt/, 'task list ssttaarrtt');
unlike ($output, qr/four/, 'task does not list four');
unlike ($output, qr/five/, 'task does not list five');
unlike ($output, qr/eenndd/, 'task does not list eenndd');
unlike ($output, qr/UUNNDDOO/, 'task does not list UUNNDDOO');

qx{../src/task rc:uuid.rc ea3b4822-574c-464b-8025-7f7be9f3cc57 modify FOUR};
$output = qx{../src/task rc:uuid.rc completed};
unlike ($output, qr/ONE/, 'task does not list ONE');
unlike ($output, qr/TWO/, 'task does not list TWO');
unlike ($output, qr/three/, 'task does not list three');
unlike ($output, qr/ssttaarrtt/, 'task does not list ssttaarrtt');
like ($output, qr/FOUR/, 'modified completed task FOUR');
like ($output, qr/five/, 'did not modify task five');
like ($output, qr/eenndd/, 'did not modify task eenndd');
like ($output, qr/UUNNDDOO/, 'did not modify task UUNNDDOO');

qx{../src/task rc:uuid.rc c1361003-948e-43e8-85c8-15d28dc3c71c modify status:pending};
$output = qx{../src/task rc:uuid.rc list};
like ($output, qr/UUNNDDOO/, 'task UUNNDDOO modified status to pending');
$output = qx{../src/task rc:uuid.rc completed};
unlike ($output, qr/UUNNDDOO/, 'task does not list UUNNDDOO after modification');

qx{../src/task rc:uuid.rc d71d3566-7a6b-4c32-8f0b-6de75bb9397b modify start:1293753600};
$output = qx{../src/task rc:uuid.rc long};
like ($output, qr/12\/31\/2010/, 'modified start date of task ssttaarrtt');

qx{../src/task rc:uuid.rc 727baa6c-65b8-485e-a810-e133e3cd83dc modify end:1293753600};
$output = qx{../src/task rc:uuid.rc completed};
like ($output, qr/12\/31\/2010/, 'modified end date of task eenndd');

qx{../src/task rc:uuid.rc aa4abef1-1dc5-4a43-b6a0-7872df3094bb modify entry:1293667200};
qx{../src/task rc:uuid.rc aa4abef1-1dc5-4a43-b6a0-7872df3094bb modify start:1293840000};
$output = qx{../src/task rc:uuid.rc long};
like ($output, qr/12\/30\/2010/, 'modified entry date of task three');
like ($output, qr/1\/1\/2011/, 'added start date of task three with modify');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key uuid.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'uuid.rc', 'Cleanup');

exit 0;

