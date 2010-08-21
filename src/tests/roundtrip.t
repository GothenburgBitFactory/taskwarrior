#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'roundtrip.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'roundtrip.rc', 'Created roundtrip.rc');
}

# Add two tasks.
qx{../task rc:roundtrip.rc add priority:H project:A one};
qx{../task rc:roundtrip.rc add +tag1 +tag2 two};

# trip 1.
qx{../task rc:roundtrip.rc export.yaml > ./roundtrip.txt};
qx{../task rc:roundtrip.rc 1,2 delete};
qx{../task rc:roundtrip.rc ls};
qx{../task rc:roundtrip.rc import ./roundtrip.txt};

# trip 2.
qx{../task rc:roundtrip.rc export.yaml > ./roundtrip.txt};
qx{../task rc:roundtrip.rc 1,2 delete};
qx{../task rc:roundtrip.rc ls};
qx{../task rc:roundtrip.rc import ./roundtrip.txt};

# Exammine.

# ID Project Pri Added    Started Due Recur Countdown Age Deps Tags      Description
# -- ------- --- -------- ------- --- ----- --------- --- ---- --------- ---------
#  1 A       H   8/7/2010                               -                one
#  2             8/7/2010                               -      tag1 tag2 two
my $output = qx{../task rc:roundtrip.rc long};
like ($output, qr/1.+A.+H.+\d+\/\d+\/\d+.+(?:-|\d+).+one/,       '2 round trips task 1 identical');
like ($output, qr/2.+\d+\/\d+\/\d+.+(?:-|\d+).+tag1\stag2\stwo/, '2 round trips task 2 identical');

# Cleanup.
unlink 'roundtrip.txt';
ok (!-r 'roundtrip.txt', 'Removed roundtrip.txt');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'roundtrip.rc';
ok (!-r 'roundtrip.rc', 'Removed roundtrip.rc');

exit 0;

