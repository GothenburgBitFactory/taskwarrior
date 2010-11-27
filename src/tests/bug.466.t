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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', '466.rc')
{
  print $fh "data.location=.\n",
            "report.foo.description=DESC\n",
            "report.foo.columns=id,description\n",
            "report.foo.sort=id,description\n";

  close $fh;
  ok (-r '466.rc', 'Created 466.rc');
}

# Bug #466 - wrong error message when sort key missing +/-
my $output = qx{../task rc:466.rc foo};
like ($output, qr/Sort column 'id' does not have a \+\/- ascending\/descending indicator\./,
      'Error on missing sort direction');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink '466.rc';
ok (!-r '466.rc', 'Removed 466.rc');

exit 0;
