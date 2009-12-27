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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'basic.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'basic.rc', 'Created basic.rc');
}

# Get the version number from configure.ac
my $version = slurp ('../../configure.ac');

# Test the usage command.
my $output = qx{../task rc:basic.rc};
like ($output, qr/You must specify a command, or a task ID to modify/m, 'missing command and ID');

# Test the version command.
$output = qx{../task rc:basic.rc version};
like ($output, qr/task $version/, 'version - task version number');
like ($output, qr/GNU General Public License/, 'version - license');
like ($output, qr/http:\/\/taskwarrior\.org/, 'version - url');

# Test the _version command.
$output = qx{../task rc:basic.rc _version};
like ($output, qr/$version/, '_version - task version number');

# Cleanup.
unlink 'basic.rc';
ok (!-r 'basic.rc', 'Removed basic.rc');

exit 0;

################################################################################
sub slurp
{
  my ($file) = @_;
  if (open my $fh, '<', $file)
  {
    while (<$fh>) {
      if (/AC_INIT/) {
        chomp;
        s/^AC_INIT\(task, //;
        s/, support.*$//;
        close  $fh;
        return $_;
      }  
    }
  }
  '';
}
