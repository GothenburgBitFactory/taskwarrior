#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 21;

# Create the rc file.
if (open my $fh, '>', 'caseless.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'caseless.rc', 'Created caseless.rc');
}

# Attempt case-sensitive and case-insensitive substitutions and filters.
qx{../src/task rc:caseless.rc add one two three};
qx{../src/task rc:caseless.rc 1 annotate four five six};
my $output;

# Description substitution.
# 2
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 /One/ONE/};
$output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/One two three/, 'one two three\nfour five six -> /One/ONE/ = fail');

# 3
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 /One/ONE/};
$output = qx{../src/task rc:caseless.rc info 1};
like ($output, qr/ONE two three/, 'one two three\nfour five six -> /One/ONE/ = caseless succeed');

# 4
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 /one/One/};
$output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/One two three/, 'ONE two three\nfour five six -> /one/ONE/ = fail');

# 5
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 /one/one/};
$output = qx{../src/task rc:caseless.rc info 1};
like ($output, qr/one two three/, 'ONE two three\nfour five six -> /one/one/ = caseless succeed');

# Annotation substitution.
# 6
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 /Five/FIVE/};
$output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/four FIVE six/, 'one two three\nfour five six -> /Five/FIVE/ = fail');

# 7
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 /Five/FIVE/};
$output = qx{../src/task rc:caseless.rc info 1};
like ($output, qr/four FIVE six/, 'one two three\nfour five six -> /Five/FIVE/ = caseless succeed');

# 8
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 /five/Five/};
$output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/four Five six/, 'one two three\nfour FIVE six -> /five/Five/ = fail');

# 9
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 /five/five/};
$output = qx{../src/task rc:caseless.rc info 1};
like ($output, qr/four five six/, 'one two three\nfour FIVE six -> /five/five/ = caseless succeed');

# Description filter.
# 10
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls One};
unlike ($output, qr/one two three/, 'one two three\nfour five six -> ls One = fail');

# 11
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls One};
like ($output, qr/one two three/, 'one two three\nfour five six -> ls One caseless = succeed');

# 12
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls Five};
unlike ($output, qr/four five six/, 'one two three\nfour five six -> ls Five = fail');

# 13
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls Five};
like ($output, qr/four five six/, 'one two three\nfour five six -> ls Five caseless = succeed');

# Annotation filter.
# 14
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls description.contains:Three};
unlike ($output, qr/one two three/, 'one two three\nfour five six -> ls description.contains:Three = fail');

# 15
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls description.contains:Three};
like ($output, qr/one two three/, 'one two three\nfour five six -> ls description.contains:Three caseless = succeed');

# 16
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls description.contains:Six};
unlike ($output, qr/four five six/, 'one two three\nfour five six -> ls description.contains:Six = fail');

# 17
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls description.contains:Six};
like ($output, qr/four five six/, 'one two three\nfour five six -> ls description.contains:Six caseless = succeed');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'caseless.rc';
ok (!-r 'caseless.rc', 'Removed caseless.rc');

exit 0;

