#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2009, Paul Beckingham.
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
use File::Path;
use Test::More tests => 8;

# Create the rc file, using rc.name:value.
unlink 'foo.rc';
rmtree 'foo', 0, 0;
qx{echo 'y'|../task rc:foo.rc rc.data.location:foo};

ok (-r 'foo.rc', 'Created default rc file');
ok (-d 'foo', 'Created default data directory');

rmtree 'foo', 0, 0;
ok (!-r 'foo', 'Removed foo');

unlink 'foo.rc';
ok (!-r 'foo.rc', 'Removed foo.rc');

# Do it all again, with rc.name=value.
qx{echo 'y'|../task rc:foo.rc rc.data.location:foo};

ok (-r 'foo.rc', 'Created default rc file');
ok (-d 'foo', 'Created default data directory');

rmtree 'foo', 0, 0;
ok (!-r 'foo', 'Removed foo');

unlink 'foo.rc';
ok (!-r 'foo.rc', 'Removed foo.rc');

exit 0;

