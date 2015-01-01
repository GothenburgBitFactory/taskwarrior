#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 56;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n";
  close $fh;
}

# Test the filters.
qx{../src/task rc:$rc add project:foo.uno priority:H +tag one foo 2>&1};
qx{../src/task rc:$rc add project:foo.dos priority:H      two 2>&1};
qx{../src/task rc:$rc add project:foo.tres                three 2>&1};
qx{../src/task rc:$rc add project:bar.uno priority:H      four 2>&1};
qx{../src/task rc:$rc add project:bar.dos            +tag five 2>&1};
qx{../src/task rc:$rc add project:bar.tres                six foo 2>&1};
qx{../src/task rc:$rc add project:bazuno                  seven bar foo 2>&1};
qx{../src/task rc:$rc add project:bazdos                  eight bar foo 2>&1};

my $output = qx{../src/task rc:$rc list 2>&1};
like   ($output, qr/one/,   'a1');
like   ($output, qr/two/,   'a2');
like   ($output, qr/three/, 'a3');
like   ($output, qr/four/,  'a4');
like   ($output, qr/five/,  'a5');
like   ($output, qr/six/,   'a6');
like   ($output, qr/seven/, 'a7');
like   ($output, qr/eight/, 'a8');

$output = qx{../src/task rc:$rc list project:foo 2>&1};
like   ($output, qr/one/,   'b1');
like   ($output, qr/two/,   'b2');
like   ($output, qr/three/, 'b3');
unlike ($output, qr/four/,  'b4');
unlike ($output, qr/five/,  'b5');
unlike ($output, qr/six/,   'b6');
unlike ($output, qr/seven/, 'b7');
unlike ($output, qr/eight/, 'b8');

$output = qx{../src/task rc:$rc list project.not:foo 2>&1};
unlike ($output, qr/one/,   'c1');
unlike ($output, qr/two/,   'c2');
unlike ($output, qr/three/, 'c3');
like   ($output, qr/four/,  'c4');
like   ($output, qr/five/,  'c5');
like   ($output, qr/six/,   'c6');
like   ($output, qr/seven/, 'c7');
like   ($output, qr/eight/, 'c8');

$output = qx{../src/task rc:$rc list project.startswith:bar 2>&1};
unlike ($output, qr/one/,   'd1');
unlike ($output, qr/two/,   'd2');
unlike ($output, qr/three/, 'd3');
like   ($output, qr/four/,  'd4');
like   ($output, qr/five/,  'd5');
like   ($output, qr/six/,   'd6');
unlike ($output, qr/seven/, 'd7');
unlike ($output, qr/eight/, 'd8');

$output = qx{../src/task rc:$rc list project:ba 2>&1};
unlike ($output, qr/one/,   'f1');
unlike ($output, qr/two/,   'f2');
unlike ($output, qr/three/, 'f3');
like   ($output, qr/four/,  'f4');
like   ($output, qr/five/,  'f5');
like   ($output, qr/six/,   'f6');
like   ($output, qr/seven/, 'f7');
like   ($output, qr/eight/, 'f8');

$output = qx{../src/task rc:$rc list project.not:ba 2>&1};
like   ($output, qr/one/,   'g1');
like   ($output, qr/two/,   'g2');
like   ($output, qr/three/, 'g3');
unlike ($output, qr/four/,  'g4');
unlike ($output, qr/five/,  'g5');
unlike ($output, qr/six/,   'g6');
unlike ($output, qr/seven/, 'g7');
unlike ($output, qr/eight/, 'g8');

$output = qx{../src/task rc:$rc list description.has:foo 2>&1};
like   ($output, qr/one/,   'i1');
unlike ($output, qr/two/,   'i2');
unlike ($output, qr/three/, 'i3');
unlike ($output, qr/four/,  'i4');
unlike ($output, qr/five/,  'i5');
like   ($output, qr/six/,   'i6');
like   ($output, qr/seven/, 'i7');
like   ($output, qr/eight/, 'i8');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

