#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 18;

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

# Description substitution.
# 2
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /One/ONE/};
my $output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/One two three/, 'one two three\nfour five six -> /One/ONE/ = fail');

# 3
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /One/ONE/};
$output = qx{../src/task rc:caseless.rc info 1};
like ($output, qr/ONE two three/, 'one two three\nfour five six -> /One/ONE/ = caseless succeed');

# 4
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /one/One/};
$output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/One two three/, 'ONE two three\nfour five six -> /one/ONE/ = fail');

# 5
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /one/one/};
$output = qx{../src/task rc:caseless.rc info 1};
like ($output, qr/one two three/, 'ONE two three\nfour five six -> /one/one/ = caseless succeed');

# Annotation substitution.
# 6
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /Five/FIVE/};
$output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/four FIVE six/, 'one two three\nfour five six -> /Five/FIVE/ = fail');

# 7
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /Five/FIVE/};
$output = qx{../src/task rc:caseless.rc info 1};
like ($output, qr/four FIVE six/, 'one two three\nfour five six -> /Five/FIVE/ = caseless succeed');

# 8
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /five/Five/};
$output = qx{../src/task rc:caseless.rc info 1};
unlike ($output, qr/four Five six/, 'one two three\nfour FIVE six -> /five/Five/ = fail');

# 9
qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /five/five/};
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
unlink qw(pending.data completed.data undo.data backlog.data synch.key caseless.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'caseless.rc', 'Cleanup');

exit 0;

