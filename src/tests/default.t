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
use Test::More tests => 18;

# Create the rc file.
if (open my $fh, '>', 'default.rc')
{
  print $fh "data.location=.\n",
            "default.command=list\n",
            "default.project=PROJECT\n",
            "default.priority=M\n";
  close $fh;
  ok (-r 'default.rc', 'Created default.rc');
}

# Set up a default command, project and priority.
qx{../task rc:default.rc add all defaults};
my $output = qx{../task rc:default.rc list};
like ($output, qr/ all defaults/, 'task added');
like ($output, qr/ PROJECT /,     'default project added');
like ($output, qr/ M /,           'default priority added');
unlink 'pending.data';

qx{../task rc:default.rc add project:specific priority:L all specified};
$output = qx{../task rc:default.rc list};
like ($output, qr/ all specified/, 'task added');
like ($output, qr/ specific /,     'project specified');
like ($output, qr/ L /,            'priority specified');
unlink 'pending.data';

qx{../task rc:default.rc add project:specific project specified};
$output = qx{../task rc:default.rc list};
like ($output, qr/ project specified/, 'task added');
like ($output, qr/ specific /,         'project specified');
like ($output, qr/ M /,                'default priority added');
unlink 'pending.data';

qx{../task rc:default.rc add priority:L priority specified};
$output = qx{../task rc:default.rc list};
like ($output, qr/ priority specified/, 'task added');
like ($output, qr/ PROJECT /,           'default project added');
like ($output, qr/ L /,                 'priority specified');

$output = qx{../task rc:default.rc};
like ($output, qr/1 PROJECT L .+ priority specified/, 'default command worked');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'default.rc';
ok (!-r 'default.rc', 'Removed default.rc');

exit 0;

