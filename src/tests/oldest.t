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
use Test::More tests => 25;

# Create the rc file.
if (open my $fh, '>', 'oldest.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'oldest.rc', 'Created oldest.rc');
}

# Add 11 tasks.  Oldest should show 1-10, newest should show 2-11.
diag ("Adding 11 tasks - takes 10 seconds");
qx{../task rc:oldest.rc add one; sleep 1};
qx{../task rc:oldest.rc add two; sleep 1};
qx{../task rc:oldest.rc add three; sleep 1};
qx{../task rc:oldest.rc add four; sleep 1};
qx{../task rc:oldest.rc add five; sleep 1};
qx{../task rc:oldest.rc add six; sleep 1};
qx{../task rc:oldest.rc add seven; sleep 1};
qx{../task rc:oldest.rc add eight; sleep 1};
qx{../task rc:oldest.rc add nine; sleep 1};
qx{../task rc:oldest.rc add ten; sleep 1};
qx{../task rc:oldest.rc add eleven};

my $output = qx{../task rc:oldest.rc oldest};
like ($output, qr/one/,      'oldest: one');
like ($output, qr/two/,      'oldest: two');
like ($output, qr/three/,    'oldest: three');
like ($output, qr/four/,     'oldest: four');
like ($output, qr/five/,     'oldest: five');
like ($output, qr/six/,      'oldest: six');
like ($output, qr/seven/,    'oldest: seven');
like ($output, qr/eight/,    'oldest: eight');
like ($output, qr/nine/,     'oldest: nine');
like ($output, qr/ten/,      'oldest: ten');
unlike ($output, qr/eleven/, 'no: eleven');

$output = qx{../task rc:oldest.rc newest};
unlike ($output, qr/one/,  'no: one');
like ($output, qr/two/,    'newest: two');
like ($output, qr/three/,  'newest: three');
like ($output, qr/four/,   'newest: four');
like ($output, qr/five/,   'newest: five');
like ($output, qr/six/,    'newest: six');
like ($output, qr/seven/,  'newest: seven');
like ($output, qr/eight/,  'newest: eight');
like ($output, qr/nine/,   'newest: nine');
like ($output, qr/ten/,    'newest: ten');
like ($output, qr/eleven/, 'newest: eleven');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'oldest.rc';
ok (!-r 'oldest.rc', 'Removed oldest.rc');

exit 0;

