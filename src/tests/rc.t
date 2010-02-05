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
use File::Path;
use Test::More tests => 15;

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

# Add a setting.
qx{echo 'y'|../task rc:foo.rc config must_be_unique old};
my $output = qx{../task rc:foo.rc config};
like ($output, qr/^must_be_unique\s+old$/ms, 'config setting a new value');

qx{echo 'y'|../task rc:foo.rc config must_be_unique new};
$output = qx{../task rc:foo.rc config};
like ($output, qr/^must_be_unique\s+new$/ms, 'config overwriting an existing value');

qx{echo 'y'|../task rc:foo.rc config must_be_unique ''};
$output = qx{../task rc:foo.rc config};
like ($output, qr/^must_be_unique$/ms, 'config setting a blank value');

qx{echo 'y'|../task rc:foo.rc config must_be_unique};
$output = qx{../task rc:foo.rc config};
unlike ($output, qr/^must_be_unique/ms, 'config removing a value');

# 'report.:b' is designed to get past the config command checks for recognized
# names.
qx{echo 'y'|../task rc:foo.rc config -- report.:b +c};
$output = qx{../task rc:foo.rc config};
like ($output, qr/^report\.:b\s+\+c/ms, 'the -- operator is working');

# Make sure the value is accepted if it has multiple words.
qx{echo 'y'|../task rc:foo.rc config must_be_unique 'one two three'};
$output = qx{../task rc:foo.rc config};
like ($output, qr/^must_be_unique\s+one two three$/ms, 'config allows multi-word quoted values');

qx{echo 'y'|../task rc:foo.rc config must_be_unique one two three};
$output = qx{../task rc:foo.rc config};
like ($output, qr/^must_be_unique\s+one two three$/ms, 'config allows multi-word unquoted values');

rmtree 'foo', 0, 0;
ok (!-r 'foo', 'Removed foo');

unlink 'foo.rc';
ok (!-r 'foo.rc', 'Removed foo.rc');

exit 0;

