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
use Test::More tests => 11;

# Create the rc file.
if (open my $fh, '>', 'sp.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'sp.rc', 'Created sp.rc');
}

my $setup = "../task rc:sp.rc add project:abc abc;"
          . "../task rc:sp.rc add project:ab  ab;"
          . "../task rc:sp.rc add project:a   a;"
          . "../task rc:sp.rc add project:b   b;";
qx{$setup};

my $output = qx{../task rc:sp.rc list project:b};
like ($output, qr/\bb\s*$/m, 'abc,ab,a,b | b -> b');

$output = qx{../task rc:sp.rc list project:a};
like ($output, qr/\babc\s*$/m, 'abc,ab,a,b | a -> abc');
like ($output, qr/\bab\s*$/m,  'abc,ab,a,b | a -> ab');
like ($output, qr/\ba\s*$/m,   'abc,ab,a,b | a -> a');

$output = qx{../task rc:sp.rc list project:ab};
like ($output, qr/\babc\s*$/m, 'abc,ab,a,b | a -> abc');
like ($output, qr/\bab\s*$/m,  'abc,ab,a,b | a -> ab');

$output = qx{../task rc:sp.rc list project:abc};
like ($output, qr/\babc\s*$/m, 'abc,ab,a,b | a -> abc');

$output = qx{../task rc:sp.rc list project:abcd};
like ($output, qr/^No matches.$/, 'abc,ab,a,b | abcd -> nul');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'sp.rc';
ok (!-r 'sp.rc', 'Removed sp.rc');

exit 0;

