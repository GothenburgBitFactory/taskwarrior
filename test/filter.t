#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 161;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'filter.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

# Test the filters.
qx{../src/task rc:filter.rc add project:A priority:H +tag one foo 2>&1};
qx{../src/task rc:filter.rc add project:A priority:H      two 2>&1};
qx{../src/task rc:filter.rc add project:A                 three 2>&1};
qx{../src/task rc:filter.rc add           priority:H      four 2>&1};
qx{../src/task rc:filter.rc add                      +tag five 2>&1};
qx{../src/task rc:filter.rc add                           six foo 2>&1};
qx{../src/task rc:filter.rc add           priority:L      seven bar foo 2>&1};

my $output = qx{../src/task rc:filter.rc list 2>&1};
like   ($output, qr/one/,   'a1');
like   ($output, qr/two/,   'a2');
like   ($output, qr/three/, 'a3');
like   ($output, qr/four/,  'a4');
like   ($output, qr/five/,  'a5');
like   ($output, qr/six/,   'a6');
like   ($output, qr/seven/, 'a7');

$output = qx{../src/task rc:filter.rc list project:A 2>&1};
like   ($output, qr/one/,   'b1');
like   ($output, qr/two/,   'b2');
like   ($output, qr/three/, 'b3');
unlike ($output, qr/four/,  'b4');
unlike ($output, qr/five/,  'b5');
unlike ($output, qr/six/,   'b6');
unlike ($output, qr/seven/, 'b7');

$output = qx{../src/task rc:filter.rc list priority:H 2>&1};
like   ($output, qr/one/,   'c1');
like   ($output, qr/two/,   'c2');
unlike ($output, qr/three/, 'c3');
like   ($output, qr/four/,  'c4');
unlike ($output, qr/five/,  'c5');
unlike ($output, qr/six/,   'c6');
unlike ($output, qr/seven/, 'c7');

$output = qx{../src/task rc:filter.rc list priority: 2>&1};
unlike ($output, qr/one/,   'd1');
unlike ($output, qr/two/,   'd2');
like   ($output, qr/three/, 'd3');
unlike ($output, qr/four/,  'd4');
like   ($output, qr/five/,  'd5');
like   ($output, qr/six/,   'd6');
unlike ($output, qr/seven/, 'd7');

$output = qx{../src/task rc:filter.rc list /foo/ 2>&1};
like   ($output, qr/one/,   'e1');
unlike ($output, qr/two/,   'e2');
unlike ($output, qr/three/, 'e3');
unlike ($output, qr/four/,  'e4');
unlike ($output, qr/five/,  'e5');
like   ($output, qr/six/,   'e6');
like   ($output, qr/seven/, 'e7');

$output = qx{../src/task rc:filter.rc list /foo/ /bar/ 2>&1};
unlike ($output, qr/one/,   'f1');
unlike ($output, qr/two/,   'f2');
unlike ($output, qr/three/, 'f3');
unlike ($output, qr/four/,  'f4');
unlike ($output, qr/five/,  'f5');
unlike ($output, qr/six/,   'f6');
like   ($output, qr/seven/, 'f7');

$output = qx{../src/task rc:filter.rc list +tag 2>&1};
like   ($output, qr/one/,   'g1');
unlike ($output, qr/two/,   'g2');
unlike ($output, qr/three/, 'g3');
unlike ($output, qr/four/,  'g4');
like   ($output, qr/five/,  'g5');
unlike ($output, qr/six/,   'g6');
unlike ($output, qr/seven/, 'g7');

$output = qx{../src/task rc:filter.rc list -tag 2>&1};
unlike ($output, qr/one/,   'h1');
like   ($output, qr/two/,   'h2');
like   ($output, qr/three/, 'h3');
like   ($output, qr/four/,  'h4');
unlike ($output, qr/five/,  'h5');
like   ($output, qr/six/,   'h6');
like   ($output, qr/seven/, 'h7');

$output = qx{../src/task rc:filter.rc list -missing 2>&1};
like   ($output, qr/one/,   'i1');
like   ($output, qr/two/,   'i2');
like   ($output, qr/three/, 'i3');
like   ($output, qr/four/,  'i4');
like   ($output, qr/five/,  'i5');
like   ($output, qr/six/,   'i6');
like   ($output, qr/seven/, 'i7');

$output = qx{../src/task rc:filter.rc list +tag -tag 2>&1};
unlike ($output, qr/one/,   'j1');
unlike ($output, qr/two/,   'j2');
unlike ($output, qr/three/, 'j3');
unlike ($output, qr/four/,  'j4');
unlike ($output, qr/five/,  'j5');
unlike ($output, qr/six/,   'j6');
unlike ($output, qr/seven/, 'j7');

$output = qx{../src/task rc:filter.rc list project:A priority:H 2>&1};
like   ($output, qr/one/,   'k1');
like   ($output, qr/two/,   'k2');
unlike ($output, qr/three/, 'k3');
unlike ($output, qr/four/,  'k4');
unlike ($output, qr/five/,  'k5');
unlike ($output, qr/six/,   'k6');
unlike ($output, qr/seven/, 'k7');

$output = qx{../src/task rc:filter.rc list project:A priority: 2>&1};
unlike ($output, qr/one/,   'l1');
unlike ($output, qr/two/,   'l2');
like   ($output, qr/three/, 'l3');
unlike ($output, qr/four/,  'l4');
unlike ($output, qr/five/,  'l5');
unlike ($output, qr/six/,   'l6');
unlike ($output, qr/seven/, 'l7');

$output = qx{../src/task rc:filter.rc list project:A /foo/ 2>&1};
like   ($output, qr/one/,   'm1');
unlike ($output, qr/two/,   'm2');
unlike ($output, qr/three/, 'm3');
unlike ($output, qr/four/,  'm4');
unlike ($output, qr/five/,  'm5');
unlike ($output, qr/six/,   'm6');
unlike ($output, qr/seven/, 'm7');

$output = qx{../src/task rc:filter.rc list project:A +tag 2>&1};
like   ($output, qr/one/,   'n1');
unlike ($output, qr/two/,   'n2');
unlike ($output, qr/three/, 'n3');
unlike ($output, qr/four/,  'n4');
unlike ($output, qr/five/,  'n5');
unlike ($output, qr/six/,   'n6');
unlike ($output, qr/seven/, 'n7');

$output = qx{../src/task rc:filter.rc list project:A priority:H /foo/ 2>&1};
like   ($output, qr/one/,   'o1');
unlike ($output, qr/two/,   'o2');
unlike ($output, qr/three/, 'o3');
unlike ($output, qr/four/,  'o4');
unlike ($output, qr/five/,  'o5');
unlike ($output, qr/six/,   'o6');
unlike ($output, qr/seven/, 'o7');

$output = qx{../src/task rc:filter.rc list project:A priority:H +tag 2>&1};
like   ($output, qr/one/,   'p1');
unlike ($output, qr/two/,   'p2');
unlike ($output, qr/three/, 'p3');
unlike ($output, qr/four/,  'p4');
unlike ($output, qr/five/,  'p5');
unlike ($output, qr/six/,   'p6');
unlike ($output, qr/seven/, 'p7');

$output = qx{../src/task rc:filter.rc list project:A priority:H /foo/ +tag 2>&1};
like   ($output, qr/one/,   'q1');
unlike ($output, qr/two/,   'q2');
unlike ($output, qr/three/, 'q3');
unlike ($output, qr/four/,  'q4');
unlike ($output, qr/five/,  'q5');
unlike ($output, qr/six/,   'q6');
unlike ($output, qr/seven/, 'q7');

$output = qx{../src/task rc:filter.rc list project:A priority:H /foo/ +tag /baz/ 2>&1};
unlike ($output, qr/one/,   'r1');
unlike ($output, qr/two/,   'r2');
unlike ($output, qr/three/, 'r3');
unlike ($output, qr/four/,  'r4');
unlike ($output, qr/five/,  'r5');
unlike ($output, qr/six/,   'r6');
unlike ($output, qr/seven/, 'r7');

# Regex filters.
#$output = qx{../src/task rc:filter.rc list rc.regex:on project:/[A-Z]/ 2>&1};
#like   ($output, qr/one/,   's1');
#like   ($output, qr/two/,   's2');
#like   ($output, qr/three/, 's3');
#unlike ($output, qr/four/,  's4');
#unlike ($output, qr/five/,  's5');
#unlike ($output, qr/six/,   's6');
#unlike ($output, qr/seven/, 's7');

#$output = qx{../src/task rc:filter.rc list rc.regex:on project:. 2>&1};
#like   ($output, qr/one/,   't1');
#like   ($output, qr/two/,   't2');
#like   ($output, qr/three/, 't3');
#unlike ($output, qr/four/,  't4');
#unlike ($output, qr/five/,  't5');
#unlike ($output, qr/six/,   't6');
#unlike ($output, qr/seven/, 't7');

$output = qx{../src/task rc:filter.rc rc.regex:on list /fo\{2\}/ 2>&1};
like   ($output, qr/one/,   'u1');
unlike ($output, qr/two/,   'u2');
unlike ($output, qr/three/, 'u3');
unlike ($output, qr/four/,  'u4');
unlike ($output, qr/five/,  'u5');
like   ($output, qr/six/,   'u6');
like   ($output, qr/seven/, 'u7');

$output = qx{../src/task rc:filter.rc rc.regex:on list /f../ /b../ 2>&1};
unlike ($output, qr/one/,   'v1');
unlike ($output, qr/two/,   'v2');
unlike ($output, qr/three/, 'v3');
unlike ($output, qr/four/,  'v4');
unlike ($output, qr/five/,  'v5');
unlike ($output, qr/six/,   'v6');
like   ($output, qr/seven/, 'v7');

$output = qx{../src/task rc:filter.rc rc.regex:on list /\\^s/ 2>&1};
unlike ($output, qr/one/,   'w1');
unlike ($output, qr/two/,   'w2');
unlike ($output, qr/three/, 'w3');
unlike ($output, qr/four/,  'w4');
unlike ($output, qr/five/,  'w5');
like   ($output, qr/six/,   'w6');
like   ($output, qr/seven/, 'w7');

$output = qx{../src/task rc:filter.rc rc.regex:on list /\\^.i/ 2>&1};
unlike ($output, qr/one/,   'x1');
unlike ($output, qr/two/,   'x2');
unlike ($output, qr/three/, 'x3');
unlike ($output, qr/four/,  'x4');
like   ($output, qr/five/,  'x5');
like   ($output, qr/six/,   'x6');
unlike ($output, qr/seven/, 'x7');

$output = qx{../src/task rc:filter.rc rc.regex:on list "/two|five/" 2>&1};
unlike ($output, qr/one/,   'y1');
like   ($output, qr/two/,   'y2');
unlike ($output, qr/three/, 'y3');
unlike ($output, qr/four/,  'y4');
like   ($output, qr/five/,  'y5');
unlike ($output, qr/six/,   'y6');
unlike ($output, qr/seven/, 'y7');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data filter.rc);
exit 0;

