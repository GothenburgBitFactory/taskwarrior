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
use Test::More tests => 13;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  print $fh "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 703: /from/t/g fails to make all changes to annotations

# Setup: Add a few tasks
diag ("2 second delay");
qx{../src/task rc:bug.rc add This is a test};
qx{../src/task rc:bug.rc 1 annotate Annotation one};
sleep 1;
qx{../src/task rc:bug.rc 1 annotate Annotation two};
sleep 1;
qx{../src/task rc:bug.rc 1 annotate Annotation three};

my $output = qx{../src/task rc:bug.rc list};
like ($output, qr/This is a test/,   'original description');
like ($output, qr/Annotation one/,   'original annotation one');
like ($output, qr/Annotation two/,   'original annotation two');
like ($output, qr/Annotation three/, 'original annotation three');

qx{../src/task rc:bug.rc 1 modify /i/I/g};
$output = qx{../src/task rc:bug.rc list};
like ($output, qr/ThIs Is a test/,   'new description');
like ($output, qr/AnnotatIon one/,   'new annotation one');
like ($output, qr/AnnotatIon two/,   'new annotation two');
like ($output, qr/AnnotatIon three/, 'new annotation three');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bug.rc';
ok (!-r 'bug.rc', 'Removed bug.rc');

exit 0;

