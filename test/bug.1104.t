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
use Test::More tests => 33;

mkdir("1",       0755);
mkdir("2",       0755);
mkdir("3",       0755);
mkdir("dropbox", 0755);

ok (-e "1",       'Created directory 1/');
ok (-e "2",       'Created directory 2/');
ok (-e "3",       'Created directory 3/');
ok (-e "dropbox", 'Created directory dropbox/');

# Create the rc file.
if (open my $fh, '>', '1.rc')
{
  print $fh "data.location=1/\n";
  print $fh "confirmation=no\n";
  print $fh "merge.autopush=yes\n";
  print $fh "merge.default.uri=./dropbox/\n";
  print $fh "push.default.uri=./dropbox/\n";
  print $fh "pull.default.uri=./dropbox/\n";

  close $fh;
  ok (-r '1.rc', 'Created 1.rc');
}

# Create the rc file.
if (open my $fh, '>', '2.rc')
{
  print $fh "data.location=2/\n";
  print $fh "confirmation=no\n";
  print $fh "merge.autopush=yes\n";
  print $fh "merge.default.uri=./dropbox/\n";
  print $fh "push.default.uri=./dropbox/\n";
  print $fh "pull.default.uri=./dropbox/\n";

  close $fh;
  ok (-r '2.rc', 'Created 2.rc');
}

# Create the rc file.
if (open my $fh, '>', '3.rc')
{
  print $fh "data.location=3/\n";
  print $fh "confirmation=no\n";
  print $fh "merge.autopush=yes\n";
  print $fh "merge.default.uri=./dropbox/\n";
  print $fh "push.default.uri=./dropbox/\n";
  print $fh "pull.default.uri=./dropbox/\n";

  close $fh;
  ok (-r '3.rc', 'Created 3.rc');
}

# Once-only push from 1 --> dropbox
my $output = qx{../src/task rc:1.rc add one 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:1.rc add two 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:1.rc add three 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:1.rc push 2>&1};
ok ($? == 0, 'Exit status check');

# Merges to 2 and 3
$output = qx{../src/task rc:2.rc merge 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:3.rc merge 2>&1};
ok ($? == 0, 'Exit status check');

# Make a different change in both locations
$output = qx{../src/task rc:1.rc add four 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:1.rc four done 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:2.rc one done 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:3.rc three delete 2>&1};
ok ($? == 0, 'Exit status check');

# Merges 1 and 2
$output = qx{../src/task rc:1.rc merge 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:2.rc merge 2>&1};
ok ($? == 0, 'Exit status check');

# see if undo.data is corrupt
$output = qx{cat 2/undo.data};
unlike ($output, qr/time 0/, "undo.data corrupt");

# merge again in order to stimulate duplications
$output = qx{../src/task rc:1.rc merge 2>&1};
ok ($? == 0, 'Exit status check');
$output = qx{../src/task rc:1.rc diag 2>&1};
unlike ($output, qr/Found duplicate/, "Found duplicate");

# Merges 3
$output = qx{../src/task rc:3.rc merge 2>&1};
ok ($? == 0, 'Exit status check');
unlike ($output, qr/Retaining/, "Must not retain changes");

# Merges 1
$output = qx{../src/task rc:1.rc merge 2>&1};
ok ($? == 0, 'Exit status check');
unlike ($output, qr/Retaining/, "Must not retain changes");

# Merges 1
$output = qx{../src/task rc:2.rc merge 2>&1};
ok ($? == 0, 'Exit status check');
unlike ($output, qr/Retaining/, "Must not retain changes");

# now all three instances must be in sync
$output = qx{diff 1/undo.data dropbox/undo.data};
ok ($? == 0, 'Resource 1 up-to-date check');

$output = qx{diff 2/undo.data dropbox/undo.data};
ok ($? == 0, 'Resource 2 up-to-date check');

$output = qx{diff 3/undo.data dropbox/undo.data};
ok ($? == 0, 'Resource 3 up-to-date check');

## Merges 3
#$output = qx{../src/task rc:3.rc merge 2>&1};
#ok ($? == 0, 'Exit status check');
#unlike ($output, qr/Retaining/, "Must not retain changes");
#
## Merges 1
#$output = qx{../src/task rc:1.rc merge 2>&1};
#ok ($? == 0, 'Exit status check');
#unlike ($output, qr/Retaining/, "Must not retain changes");
#
## Merges 1
#$output = qx{../src/task rc:2.rc merge 2>&1};
#ok ($? == 0, 'Exit status check');
#unlike ($output, qr/Retaining/, "Must not retain changes");

# Cleanup.
unlink qw(1.rc 1/pending.data 1/completed.data 1/undo.data 1/backlog.data 2/pending.data 2/completed.data 2/undo.data 2.rc 2/backlog.data dropbox/completed.data dropbox/pending.data dropbox/undo.data 3/pending.data 3/undo.data 3/completed.data 3/backlog.data 3.rc);
ok (! -r '1/pending.data'         &&
    ! -r '1/completed.data'       &&
    ! -r '1/undo.data'            &&
    ! -r '1/backlog.data'         &&
    ! -r '1.rc'                   &&
    ! -r '2/pending.data'         &&
    ! -r '2/completed.data'       &&
    ! -r '2/undo.data'            &&
    ! -r '2/backlog.data'         &&
    ! -r '2.rc'                   &&
    ! -r '3/pending.data'         &&
    ! -r '3/completed.data'       &&
    ! -r '3/undo.data'            &&
    ! -r '3/backlog.data'         &&
    ! -r '3.rc'                   &&
    ! -r 'dropbox/pending.data'   &&
    ! -r 'dropbox/completed.data' &&
    ! -r 'dropbox/undo.data' , 'Cleanup');

rmtree (['1', '2', '3', 'dropbox'], 0, 1);
ok (! -e '1'             &&
    ! -e '2'             &&
    ! -e '3'             &&
    ! -e 'dropbox', 'Removed directories');

exit 0;
