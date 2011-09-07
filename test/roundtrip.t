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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'roundtrip.rc')
{
  print $fh "data.location=.\n",
            "verbose=off\n",
            "confirmation=no\n",
            "defaultwidth=100\n";
  close $fh;
  ok (-r 'roundtrip.rc', 'Created roundtrip.rc');
}

# Add two tasks.
qx{../src/task rc:roundtrip.rc add priority:H project:A one};
qx{../src/task rc:roundtrip.rc add +tag1 +tag2 two};

# trip 1.
qx{../src/task rc:roundtrip.rc export > ./roundtrip.txt};
unlink 'pending.data', 'completed.data', 'undo.data';
qx{../src/task rc:roundtrip.rc rc.debug:1 import ./roundtrip.txt};

# trip 2.
qx{../src/task rc:roundtrip.rc export > ./roundtrip.txt};
unlink 'pending.data', 'completed.data', 'undo.data';
qx{../src/task rc:roundtrip.rc import ./roundtrip.txt};

# Exammine.

# ID Project Pri Added    Started Due Recur Countdown Age Deps Tags      Description
# -- ------- --- -------- ------- --- ----- --------- --- ---- --------- ---------
#  1 A       H   8/7/2010                               -                one
#  2             8/7/2010                               -      tag1 tag2 two
my $output = qx{../src/task rc:roundtrip.rc long};
like ($output, qr/1.+A.+H.+\d+\/\d+\/\d+.+(?:-|\d+).+one/,       '2 round trips task 1 identical');
like ($output, qr/2.+\d+\/\d+\/\d+.+(?:-|\d+).+tag1\stag2\stwo/, '2 round trips task 2 identical');

# Cleanup.
unlink qw(roundtrip.txt pending.data completed.data undo.data backlog.data synch.key roundtrip.rc);
ok (! -r 'roundtrip.txt'  &&
    ! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'roundtrip.rc', 'Cleanup');

exit 0;

