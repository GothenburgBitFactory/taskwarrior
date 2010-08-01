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
if (open my $fh, '>', '455.rc')
{
  print $fh "data.location=.\n";

  close $fh;
  ok (-r '455.rc', 'Created 455.rc');
}

# Bug #455 - Text alignment in reports is broken when text contains utf8 characters

qx{../task rc:455.rc add abc pro:Bar\x{263A}};
qx{../task rc:455.rc add def pro:Foo!};

my $output = qx{../task rc:455.rc ls};

like ($output, qr/\s{7}abc/ms, 'bug 455 - correct spacing in utf8 task');
like ($output, qr/\s{7}def/ms, 'bug 455 - correct spacing in non utf8 task');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink '455.rc';
ok (!-r '455.rc', 'Removed 455.rc');

exit 0;
