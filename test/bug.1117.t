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
use File::Copy;
use File::Path;
use Test::More tests => 13;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

mkdir("1",       0755);
mkdir("2",       0755);
mkdir("merge",   0755);

ok (-e "1",       'Created directory 1/');
ok (-e "2",       'Created directory 2/');
ok (-e "merge",   'Created directory merge/');

# Create the rc file.
if (open my $fh, '>', '1.rc')
{
  print $fh "data.location=1/\n";
  print $fh "confirmation=no\n";
  print $fh "merge.autopush=yes\n";
  print $fh "merge.default.uri=./merge/\n";
  print $fh "push.default.uri=./merge/\n";

  close $fh;
  ok (-r '1.rc', 'Created 1.rc');
}

# Create the rc file.
if (open my $fh, '>', '2.rc')
{
  print $fh "data.location=2/\n";
  print $fh "confirmation=no\n";
  print $fh "merge.autopush=yes\n";
  print $fh "merge.default.uri=./merge/\n";
  print $fh "push.default.uri=./merge/\n";

  close $fh;
  ok (-r '2.rc', 'Created 2.rc');
}

# add and push on 1
my $output = qx{../src/task rc:1.rc add foo1 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:1.rc push};
ok ($? == 0, 'Exit status check');

# add and merge on 2
$output = qx{../src/task rc:2.rc add foo2 2>&1};
ok ($? == 0, 'Exit status check');

$output = qx{../src/task rc:2.rc merge 2>&1};
ok ($? == 0, 'Exit status check');

# merge 1
$output = qx{../src/task rc:1.rc merge 2>&1};
ok ($? == 0, 'Exit status check');

# undo.data files must not differ
$output = qx{diff 1/undo.data 2/undo.data};
ok ($? == 0, 'undo.data diff result');

# Cleanup.
unlink qw(1.rc 1/pending.data 1/completed.data 1/undo.data 1/backlog.data 1/synch.key 2/pending.data 2/completed.data 2/undo.data 2.rc 2/backlog.data 2/synch.key merge/completed.data merge/pending.data merge/undo.data);
ok (! -r '1/pending.data'         &&
    ! -r '1/completed.data'       &&
    ! -r '1/undo.data'            &&
    ! -r '1/backlog.data'         &&
    ! -r '1/synch.key'            &&
    ! -r '1.rc'                   &&
    ! -r '2/pending.data'         &&
    ! -r '2/completed.data'       &&
    ! -r '2/undo.data'            &&
    ! -r '2/backlog.data'         &&
    ! -r '2/synch.key'            &&
    ! -r '2.rc'                   &&
    ! -r 'merge/pending.data'   &&
    ! -r 'merge/completed.data' &&
    ! -r 'merge/undo.data' , 'Cleanup');

rmtree (['1', '2', 'merge'], 0, 1);
ok (! -e '1'             &&
    ! -e '2'             &&
    ! -e 'merge', 'Removed directories');

exit 0;
