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
use Test::More tests => 16;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'caseless.rc')
{
  print $fh "data.location=.\n",
            "report.ls.columns=id,project,priority,description\n",
            "report.ls.labels=ID,Proj,Pri,Description\n",
            "report.ls.sort=priority-,project+\n",
            "report.ls.filter=status:pending\n";

  close $fh;
}

# Attempt case-sensitive and case-insensitive substitutions and filters.
qx{../src/task rc:caseless.rc add one two three 2>&1};
qx{../src/task rc:caseless.rc 1 annotate four five six 2>&1};

# Description substitution.
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /One/ONE/ 2>&1};
my $output = qx{../src/task rc:caseless.rc info 1 2>&1};
unlike ($output, qr/One two three/, 'one two three\nfour five six -> /One/ONE/ = fail');

qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /One/ONE/ 2>&1};
$output = qx{../src/task rc:caseless.rc info 1 2>&1};
like ($output, qr/ONE two three/, 'one two three\nfour five six -> /One/ONE/ = caseless succeed');

qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /one/One/ 2>&1};
$output = qx{../src/task rc:caseless.rc info 1 2>&1};
unlike ($output, qr/One two three/, 'ONE two three\nfour five six -> /one/ONE/ = fail');

qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /one/one/ 2>&1};
$output = qx{../src/task rc:caseless.rc info 1 2>&1};
like ($output, qr/one two three/, 'ONE two three\nfour five six -> /one/one/ = caseless succeed');

# Annotation substitution.
qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /Five/FIVE/ 2>&1};
$output = qx{../src/task rc:caseless.rc info 1 2>&1};
unlike ($output, qr/four FIVE six/, 'one two three\nfour five six -> /Five/FIVE/ = fail');

qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /Five/FIVE/ 2>&1};
$output = qx{../src/task rc:caseless.rc info 1 2>&1};
like ($output, qr/four FIVE six/, 'one two three\nfour five six -> /Five/FIVE/ = caseless succeed');

qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes 1 modify /five/Five/ 2>&1};
$output = qx{../src/task rc:caseless.rc info 1 2>&1};
unlike ($output, qr/four Five six/, 'one two three\nfour FIVE six -> /five/Five/ = fail');

qx{../src/task rc:caseless.rc rc.search.case.sensitive:no 1 modify /five/five/ 2>&1};
$output = qx{../src/task rc:caseless.rc info 1 2>&1};
like ($output, qr/four five six/, 'one two three\nfour FIVE six -> /five/five/ = caseless succeed');

# Description filter.
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls /One/ 2>&1};
unlike ($output, qr/one two three/, 'one two three\nfour five six -> ls /One/ = fail');

$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls /One/ 2>&1};
like ($output, qr/one two three/, 'one two three\nfour five six -> ls One caseless = succeed');

$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls /Five/ 2>&1};
unlike ($output, qr/four five six/, 'one two three\nfour five six -> ls /Five/ = fail');

$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls /Five/ 2>&1};
like ($output, qr/four five six/, 'one two three\nfour five six -> ls /Five/ caseless = succeed');

# Annotation filter.
$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls description.contains:Three 2>&1};
unlike ($output, qr/one two three/, 'one two three\nfour five six -> ls description.contains:Three = fail');

$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls description.contains:Three 2>&1};
like ($output, qr/one two three/, 'one two three\nfour five six -> ls description.contains:Three caseless = succeed');

$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:yes ls description.contains:Six 2>&1};
unlike ($output, qr/four five six/, 'one two three\nfour five six -> ls description.contains:Six = fail');

$output = qx{../src/task rc:caseless.rc rc.search.case.sensitive:no ls description.contains:Six 2>&1};
like ($output, qr/four five six/, 'one two three\nfour five six -> ls description.contains:Six caseless = succeed');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data caseless.rc);
exit 0;

