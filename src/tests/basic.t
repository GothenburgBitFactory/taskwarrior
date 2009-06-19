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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'basic.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'basic.rc', 'Created basic.rc');
}

# Test the usage command.
my $output = qx{../task rc:basic.rc};
like ($output, qr/Usage: task/, 'usage');
like ($output, qr/http:\/\/taskwarrior\.org/, 'usage - url');

# Test the version command.
$output = qx{../task rc:basic.rc version};
like ($output, qr/task \d+\.\d+\.\d+/, 'version - task version number');
like ($output, qr/ABSOLUTELY NO WARRANTY/, 'version - warranty');
like ($output, qr/http:\/\/taskwarrior\.org/, 'version - url');

# Cleanup.
unlink 'basic.rc';
ok (!-r 'basic.rc', 'Removed basic.rc');

exit 0;

