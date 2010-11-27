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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'custom.rc')
{
  print $fh "data.location=.\n",
            "report.foo.description=DESC\n",
            "report.foo.columns=id,description\n",
            "report.foo.sort=id+\n",
            "report.foo.filter=project:A\n";
  close $fh;
  ok (-r 'custom.rc', 'Created custom.rc');
}

# Generate the usage screen, and locate the custom report on it.
my $output = qx{../task rc:custom.rc usage};
like ($output, qr/task foo \[tags\] \[attrs\] desc\.\.\.\s+DESC\n/m, 'report.foo');

qx{../task rc:custom.rc add project:A one};
qx{../task rc:custom.rc add two};
$output = qx{../task rc:custom.rc foo};
like ($output, qr/one/, 'custom filter included');
unlike ($output, qr/two/, 'custom filter excluded');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'custom.rc';
ok (!-r 'custom.rc', 'Removed custom.rc');

exit 0;

