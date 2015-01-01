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
use Test::More tests => 22;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'annotate.rc')
{
  # Note: Use 'rrr' to guarantee a unique report name.  Using 'r' conflicts
  #       with 'recurring'.
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "report.rrr.description=rrr\n",
            "report.rrr.columns=id,description\n",
            "report.rrr.sort=id+\n",
            "color=off\n",
            "dateformat=m/d/Y\n";
  close $fh;
}

# Add four tasks, annotate one three times, one twice, one just once and one none.
qx{../src/task rc:annotate.rc add one 2>&1};
qx{../src/task rc:annotate.rc add two 2>&1};
qx{../src/task rc:annotate.rc add three 2>&1};
qx{../src/task rc:annotate.rc add four 2>&1};
qx{../src/task rc:annotate.rc 1 annotate foo1 2>&1};
qx{../src/task rc:annotate.rc 1 annotate foo2 2>&1};
qx{../src/task rc:annotate.rc 1 annotate foo3 2>&1};
qx{../src/task rc:annotate.rc 2 annotate bar1 2>&1};
qx{../src/task rc:annotate.rc 2 annotate bar2 2>&1};
qx{../src/task rc:annotate.rc 3 annotate baz1 2>&1};

my $output = qx{../src/task rc:annotate.rc rrr 2>&1};

# ID Description
# -- -------------------------------
#  1 one
#    3/24/2009 foo1
#    3/24/2009 foo2
#    3/24/2009 foo3
#  2 two
#    3/24/2009 bar1
#    3/24/2009 bar2
#  3 three
#    3/24/2009 baz1
#  4 four
#
# 4 tasks

like ($output, qr/1 one/,   'task 1'); # 2
like ($output, qr/2 two/,   'task 2');
like ($output, qr/3 three/, 'task 3');
like ($output, qr/4 four/,  'task 4');
like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4}\s+foo1/ms,  'full - first  annotation task 1');
like ($output, qr/foo1.+\d{1,2}\/\d{1,2}\/\d{4}\s+foo2/ms, 'full - second annotation task 1');
like ($output, qr/foo2.+\d{1,2}\/\d{1,2}\/\d{4}\s+foo3/ms, 'full - third  annotation task 1');
like ($output, qr/two.+\d{1,2}\/\d{1,2}\/\d{4}\s+bar1/ms,  'full - first  annotation task 2');
like ($output, qr/bar1.+\d{1,2}\/\d{1,2}\/\d{4}\s+bar2/ms, 'full - second annotation task 2');
like ($output, qr/three.+\d{1,2}\/\d{1,2}\/\d{4}\s+baz1/ms,'full - first  annotation task 3');
like ($output, qr/4 tasks/, 'count');

if (open my $fh, '>', 'annotate2.rc')
{
  # Note: Use 'rrr' to guarantee a unique report name.  Using 'r' conflicts
  #       with 'recurring'.
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "report.rrr.description=rrr\n",
            "report.rrr.columns=id,description\n",
            "report.rrr.sort=id+\n",
            "dateformat.annotation=yMD HNS\n";
  close $fh;
}

$output = qx{../src/task rc:annotate2.rc rrr 2>&1};
like ($output, qr/1 one/,   'task 1'); # 14
like ($output, qr/2 two/,   'task 2');
like ($output, qr/3 three/, 'task 3');
like ($output, qr/4 four/,  'task 4');
like ($output, qr/one.+\d{1,6}\s+\d{1,6}\s+foo1/ms,  'dateformat - first  annotation task 1'); #18
like ($output, qr/foo1.+\d{1,6}\s+\d{1,6}\s+foo2/ms, 'dateformat - second annotation task 1');
like ($output, qr/foo2.+\d{1,6}\s+\d{1,6}\s+foo3/ms, 'dateformat - third  annotation task 1');
like ($output, qr/two.+\d{1,6}\s+\d{1,6}\s+bar1/ms,  'dateformat - first  annotation task 2');
like ($output, qr/bar1.+\d{1,6}\s+\d{1,6}\s+bar2/ms, 'dateformat - second annotation task 2');
like ($output, qr/three.+\d{1,6}\s+\d{1,6}\s+baz1/ms,'dateformat - first  annotation task 3');
like ($output, qr/4 tasks/, 'count');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data annotate.rc annotate2.rc);
exit 0;

