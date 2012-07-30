#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 54;

# Create the rc file.
if (open my $fh, '>', 'oldest.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'oldest.rc', 'Created oldest.rc');
}

# Add 11 tasks.  Oldest should show 1-10, newest should show 2-11.
qx{../src/task rc:oldest.rc add one 2>&1};
diag ("3 second delay");
sleep 1;
qx{../src/task rc:oldest.rc add two 2>&1};
sleep 1;
qx{../src/task rc:oldest.rc add three 2>&1};
sleep 1;
my $output = qx{../src/task rc:oldest.rc oldest 2>&1};
like ($output, qr/one/,               'oldest: one');
like ($output, qr/two/,               'oldest: two');
like ($output, qr/three/,             'oldest: three');
like ($output, qr/one.*two.*three/ms, 'oldest: sort');

$output = qx{../src/task rc:oldest.rc newest 2>&1};
like ($output, qr/three/,             'newest: three');
like ($output, qr/two/,               'newest: two');
like ($output, qr/one/,               'newest: one');
like ($output, qr/three.*two.*one/ms, 'newest: sort');

qx{../src/task rc:oldest.rc add four 2>&1};
diag ("7 second delay");
sleep 1;
qx{../src/task rc:oldest.rc add five 2>&1};
sleep 1;
qx{../src/task rc:oldest.rc add six 2>&1};
sleep 1;
qx{../src/task rc:oldest.rc add seven 2>&1};
sleep 1;
qx{../src/task rc:oldest.rc add eight 2>&1};
sleep 1;
qx{../src/task rc:oldest.rc add nine 2>&1};
sleep 1;
qx{../src/task rc:oldest.rc add ten 2>&1};
sleep 1;
qx{../src/task rc:oldest.rc add eleven 2>&1};

$output = qx{../src/task rc:oldest.rc oldest 2>&1};
like   ($output, qr/one/,    'oldest: one');   # 10
like   ($output, qr/two/,    'oldest: two');
like   ($output, qr/three/,  'oldest: three');
like   ($output, qr/four/,   'oldest: four');
like   ($output, qr/five/,   'oldest: five');
like   ($output, qr/six/,    'oldest: six');
like   ($output, qr/seven/,  'oldest: seven');
like   ($output, qr/eight/,  'oldest: eight');
like   ($output, qr/nine/,   'oldest: nine');
like   ($output, qr/ten/,    'oldest: ten');
unlike ($output, qr/eleven/, 'no: eleven');   # 20

$output = qx{../src/task rc:oldest.rc oldest limit:3 2>&1};
like   ($output, qr/one/,    'oldest: one');
like   ($output, qr/two/,    'oldest: two');
like   ($output, qr/three/,  'oldest: three');
unlike ($output, qr/four/,   'no: four');
unlike ($output, qr/five/,   'no: five');
unlike ($output, qr/six/,    'no: six');
unlike ($output, qr/seven/,  'no: seven');
unlike ($output, qr/eight/,  'no: eight');
unlike ($output, qr/nine/,   'no: nine');
unlike ($output, qr/ten/,    'no: ten');    # 30
unlike ($output, qr/eleven/, 'no: eleven');

$output = qx{../src/task rc:oldest.rc newest 2>&1};
unlike ($output, qr/one/,    'no: one');
like   ($output, qr/two/,    'newest: two');
like   ($output, qr/three/,  'newest: three');
like   ($output, qr/four/,   'newest: four');
like   ($output, qr/five/,   'newest: five');
like   ($output, qr/six/,    'newest: six');
like   ($output, qr/seven/,  'newest: seven');
like   ($output, qr/eight/,  'newest: eight');
like   ($output, qr/nine/,   'newest: nine');   # 40
like   ($output, qr/ten/,    'newest: ten');
like   ($output, qr/eleven/, 'newest: eleven');

$output = qx{../src/task rc:oldest.rc newest limit:3 2>&1};
unlike ($output, qr/one/,    'no: one');
unlike ($output, qr/two/,    'no: two');
unlike ($output, qr/three/,  'no: three');
unlike ($output, qr/four/,   'no: four');
unlike ($output, qr/five/,   'no: five');
unlike ($output, qr/six/,    'no: six');
unlike ($output, qr/seven/,  'no: seven');
unlike ($output, qr/eight/,  'no: eight');    # 50
like   ($output, qr/nine/,   'newest: nine');
like   ($output, qr/ten/,    'newest: ten');
like   ($output, qr/eleven/, 'newest: eleven');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key oldest.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'oldest.rc', 'Cleanup');

exit 0;

