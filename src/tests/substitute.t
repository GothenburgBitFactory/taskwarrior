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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'subst.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'subst.rc', 'Created subst.rc');
}

# Test the substitution command.
qx{../task rc:subst.rc add foo};
qx{../task rc:subst.rc 1 /foo/FOO/};
my $output = qx{../task rc:subst.rc info 1};
like ($output, qr/FOO/, 'substitution in description');

# Test the substitution command on annotations.
qx{../task rc:subst.rc annotate 1 bar};
qx{../task rc:subst.rc 1 /bar/BAR/};
$output = qx{../task rc:subst.rc info 1};
like ($output, qr/BAR/, 'substitution in annotation');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'subst.rc';
ok (!-r 'subst.rc', 'Removed subst.rc');

exit 0;

