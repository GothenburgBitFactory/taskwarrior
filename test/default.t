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
use Test::More tests => 21;

# Create the rc file.
if (open my $fh, '>', 'default.rc')
{
  print $fh "data.location=.\n",
            "default.command=list\n",
            "default.project=PROJECT\n",
            "default.priority=M\n",
            "default.due=eom\n";
  close $fh;
  ok (-r 'default.rc', 'Created default.rc');
}

# Set up a default command, project and priority.
qx{../src/task rc:default.rc add all defaults};
my $output = qx{../src/task rc:default.rc list};
like ($output, qr/ all defaults/, 'task added');
like ($output, qr/ PROJECT /,     'default project added');
like ($output, qr/ M /,           'default priority added');
like ($output, qr/\//,            'default due added');
unlink 'pending.data';

qx{../src/task rc:default.rc add project:specific priority:L due:eoy all specified};
$output = qx{../src/task rc:default.rc list};
like ($output, qr/ all specified/, 'task added');
like ($output, qr/ specific /,     'project specified');
like ($output, qr/ L /,            'priority specified');
like ($output, qr/\//,             'due specified');
unlink 'pending.data';

qx{../src/task rc:default.rc add project:specific project specified};
$output = qx{../src/task rc:default.rc list};
like ($output, qr/ project specified/, 'task added');
like ($output, qr/ specific /,         'project specified');
like ($output, qr/ M /,                'default priority added');
like ($output, qr/\//,                 'default due added');
unlink 'pending.data';

qx{../src/task rc:default.rc add priority:L priority specified};
$output = qx{../src/task rc:default.rc list};
like ($output, qr/ priority specified/, 'task added');
like ($output, qr/ PROJECT /,           'default project added');
like ($output, qr/ L /,                 'priority specified');
like ($output, qr/\//,                  'default due added');

$output = qx{../src/task rc:default.rc};
like ($output, qr/1 PROJECT L .+ priority specified/, 'default command worked');

qx{../src/task rc:default.rc add project:HOME priority:M due:tomorrow all specified};
qx{echo '-- y' | ../src/task rc:default.rc config default.command 'list priority:M'};
$output = qx{../src/task rc:default.rc};
like   ($output, qr/ M /, 'priority:M included in default command');
unlike ($output, qr/ L /, 'priority:L excluded from default command');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key default.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'default.rc', 'Cleanup');

exit 0;

