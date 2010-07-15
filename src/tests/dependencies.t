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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'dep.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'dep.rc', 'Created dep.rc');
}

# TODO t 1 dep:2; t info 1 => blocked by 2
# TODO            t info 2 => blocking 1
# TODO t 1 dep:2,3,4; t 1 dep:-2,-4,5; t info 1 => blocked by 3,5
# TODO t 1 dep:2; t 2 dep:1 => error

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'dep.rc';
ok (!-r 'dep.rc', 'Removed dep.rc');

exit 0;

