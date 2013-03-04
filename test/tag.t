#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

use strict;
use warnings;
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'tag.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'tag.rc', 'Created tag.rc');
}

# Add task with tags.
my $output = qx{../src/task rc:tag.rc add +one This +two is a test +three 2>&1; ../src/task rc:tag.rc info 1 2>&1};
like ($output, qr/^Tags\s+one two three\n/m, 'tags found');

# Remove tags.
$output = qx{../src/task rc:tag.rc 1 modify -three -two -one 2>&1; ../src/task rc:tag.rc info 1 2>&1};
unlike ($output, qr/^Tags/m, '-three -two -one tag removed');

# Add tags.
$output = qx{../src/task rc:tag.rc 1 modify +four +five +six 2>&1; ../src/task rc:tag.rc info 1 2>&1};
like ($output, qr/^Tags\s+four five six\n/m, 'tags found');

# Remove tags.
$output = qx{../src/task rc:tag.rc 1 modify -four -five -six 2>&1; ../src/task rc:tag.rc info 1 2>&1};
unlike ($output, qr/^Tags/m, '-four -five -six tag removed');

# Add and remove tags.
$output = qx{../src/task rc:tag.rc 1 modify +duplicate -duplicate 2>&1; ../src/task rc:tag.rc info 1 2>&1};
unlike ($output, qr/^Tags/m, '+duplicate -duplicate NOP');

# Remove missing tag.
$output = qx{../src/task rc:tag.rc 1 modify -missing 2>&1; ../src/task rc:tag.rc info 1 2>&1};
unlike ($output, qr/^Tags/m, '-missing NOP');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key tag.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'tag.rc', 'Cleanup');

exit 0;

