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
use Test::More tests => 24;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'seq.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "dateformat.annotation=m/d/Y\n";
  close $fh;
}

# Test sequences in done/undo
qx{../src/task rc:seq.rc add one mississippi 2>&1};
qx{../src/task rc:seq.rc add two mississippi 2>&1};
qx{../src/task rc:seq.rc 1,2 done 2>&1};
my $output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Status\s+Completed/, 'sequence done 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Status\s+Completed/, 'sequence done 2');
qx{../src/task rc:seq.rc undo 2>&1};
qx{../src/task rc:seq.rc undo 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Status\s+Pending/, 'sequence undo 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Status\s+Pending/, 'sequence undo 2');

# Test sequences in delete/undelete
qx{../src/task rc:seq.rc 1,2 delete 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Status\s+Deleted/, 'sequence delete 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Status\s+Deleted/, 'sequence delete 2');
qx{../src/task rc:seq.rc undo 2>&1};
qx{../src/task rc:seq.rc undo 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Status\s+Pending/, 'sequence undo 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Status\s+Pending/, 'sequence undo 2');

# Test sequences in start/stop
qx{../src/task rc:seq.rc 1,2 start 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Start/, 'sequence start 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Start/, 'sequence start 2');
qx{../src/task rc:seq.rc 1,2 stop 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Start\sdeleted/, 'sequence stop 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Start\sdeleted/, 'sequence stop 2');

# Test sequences in modify
qx{../src/task rc:seq.rc 1,2 modify +tag 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Tags\s+tag/, 'sequence modify 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Tags\s+tag/, 'sequence modify 2');
qx{../src/task rc:seq.rc 1,2 modify -tag 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
unlike ($output, qr/Tags\s+tag/, 'sequence unmodify 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
unlike ($output, qr/Tags\s+tag/, 'sequence unmodify 2');

# Test sequences in substitutions
qx{../src/task rc:seq.rc 1,2 modify /miss/Miss/ 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/Description\s+one Miss/, 'sequence substitution 1');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/Description\s+two Miss/, 'sequence substitution 2');

# Test sequences in info
$output = qx{../src/task rc:seq.rc info 1,2 2>&1};
like ($output, qr/Description\s+one Miss/, 'sequence info 1');
like ($output, qr/Description\s+two Miss/, 'sequence info 2');

# Test sequences in duplicate
qx{../src/task rc:seq.rc 1,2 duplicate pri:H 2>&1};
$output = qx{../src/task rc:seq.rc info 3 2>&1};
like ($output, qr/Priority\s+H/, 'sequence duplicate 1');
$output = qx{../src/task rc:seq.rc info 4 2>&1};
like ($output, qr/Priority\s+H/, 'sequence duplicate 2');

# Test sequences in annotate
qx{../src/task rc:seq.rc 1,2 annotate note 2>&1};
$output = qx{../src/task rc:seq.rc info 1 2>&1};
like ($output, qr/\d+\/\d+\/\d+ note/, 'sequence 1 annotate');
$output = qx{../src/task rc:seq.rc info 2 2>&1};
like ($output, qr/\d+\/\d+\/\d+ note/, 'sequence 2 annotate');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data seq.rc);
exit 0;

