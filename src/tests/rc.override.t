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
if (open my $fh, '>', 'rc.rc')
{
  print $fh "data.location=.\n",
            "foo=bar\n";
  close $fh;
  ok (-r 'rc.rc', 'Created rc.rc');
}

my $output = qx{../task rc:rc.rc show};
like ($output, qr/\sfoo\s+bar/, 'unmodified');

$output = qx{../task rc:rc.rc rc.foo:baz show};
like ($output, qr/\sfoo\s+baz/, 'overridden');

unlink 'rc.rc';
ok (!-r 'rc.rc', 'Removed rc.rc');

exit 0;

