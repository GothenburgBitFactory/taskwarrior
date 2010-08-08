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
use Test::More tests => 11;

# Create the rc file.
if (open my $fh, '>', 'info.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'info.rc', 'Created info.rc');
}

# Test the add command.
qx{../task rc:info.rc add test one};
qx{../task rc:info.rc add test two};
qx{../task rc:info.rc add test three};

my $output = qx{../task rc:info.rc 1};
like ($output, qr/Description\s+test one\n/, 'single auto-info one');
unlike ($output, qr/Description\s+test two\n/, 'single auto-info !two');
unlike ($output, qr/Description\s+test three\n/, 'single auto-info !three');

$output = qx{../task rc:info.rc 1-2};
like ($output, qr/Description\s+test one\n/, 'single auto-info one');
like ($output, qr/Description\s+test two\n/, 'single auto-info two');
unlike ($output, qr/Description\s+test three\n/, 'single auto-info !three');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'info.rc';
ok (!-r 'info.rc', 'Removed info.rc');

exit 0;

