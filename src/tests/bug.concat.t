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
if (open my $fh, '>', 'bug_concat.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug_concat.rc', 'Created bug_concat.rc');
}

# When a task is modified like this:
#
#   % task 1 This is a new description
#
# The arguments are concatenated thus:
#
#   Thisisanewdescription

qx{../task rc:bug_concat.rc add This is the original text};

my $output = qx{../task rc:bug_concat.rc info 1};
like ($output, qr/Description\s+This is the original text\n/, 'original correct');

qx{../task rc:bug_concat.rc 1 This is the modified text};
$output = qx{../task rc:bug_concat.rc info 1};
like ($output, qr/Description\s+This is the modified text\n/, 'modified correct');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'bug_concat.rc';
ok (!-r 'bug_concat.rc', 'Removed bug_concat.rc');

exit 0;

