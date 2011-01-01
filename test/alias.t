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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'alias.rc')
{
  print $fh "data.location=.\n",
            "alias.foo=_projects\n",
            "alias.bar=foo\n";
  close $fh;
  ok (-r 'alias.rc', 'Created alias.rc');
}

# Add a task with certain project, then access that task via aliases.
qx{../src/task rc:alias.rc add project:ALIAS foo};

my $output = qx{../src/task rc:alias.rc _projects};
like ($output, qr/ALIAS/, 'task _projects -> ALIAS');

$output = qx{../src/task rc:alias.rc foo};
like ($output, qr/ALIAS/, 'task foo -> _projects -> ALIAS');

$output = qx{../src/task rc:alias.rc bar};
like ($output, qr/ALIAS/, 'task bar -> foo -> _projects -> ALIAS');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'alias.rc';
ok (!-r 'alias.rc', 'Removed alias.rc');

exit 0;

