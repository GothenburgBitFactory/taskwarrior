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
use Test::More tests => 25;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  # Note: Use 'rrr' to guarantee a unique report name.  Using 'r' conflicts
  #       with 'recurring'.
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "report.rrr.description=rrr\n",
            "report.rrr.columns=id,description\n",
            "report.rrr.sort=id+\n",
            "dateformat=m/d/Y\n";
  close $fh;
}

# Add four tasks, annotate one three times, one twice, one just once and one none.
qx{../src/task rc:$rc add one 2>&1};
qx{../src/task rc:$rc 1 annotate Ernie 2>&1};
qx{../src/task rc:$rc 1 annotate Bert 2>&1};
qx{../src/task rc:$rc 1 annotate Bibo 2>&1};
qx{../src/task rc:$rc 1 annotate Kermit the frog 2>&1};
qx{../src/task rc:$rc 1 annotate Kermit the frog 2>&1};
qx{../src/task rc:$rc 1 annotate Kermit 2>&1};
qx{../src/task rc:$rc 1 annotate Kermit and Miss Piggy 2>&1};

my $output = qx{../src/task rc:$rc rrr 2>&1};
like ($output, qr/1 one/,                                                   "$ut: task 1");
like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} Ernie/ms,                    "$ut: first   annotation");
like ($output, qr/Ernie.+\d{1,2}\/\d{1,2}\/\d{4} Bert/ms,                   "$ut: second  annotation");
like ($output, qr/Bert.+\d{1,2}\/\d{1,2}\/\d{4} Bibo/ms,                    "$ut: third   annotation");
like ($output, qr/Bibo.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,         "$ut: fourth  annotation"); # 5
like ($output, qr/frog.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,         "$ut: fifth   annotation");
like ($output, qr/frog.+\d{1,2}\/\d{1,2}\/\d{4} Kermit/ms,                  "$ut: sixth   annotation");
like ($output, qr/Kermit.+\d{1,2}\/\d{1,2}\/\d{4} Kermit and Miss Piggy/ms, "$ut: seventh annotation");
like ($output, qr/1 task/,                                                  "$ut: count");

qx{../src/task rc:$rc 1 denotate Ernie 2>&1};
$output = qx{../src/task rc:$rc rrr 2>&1};
unlike ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} Ernie/ms,                  "$ut: Delete annotation"); # 10
like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} Bert/ms,                     "$ut: Bert now first annotationt");

qx{../src/task rc:$rc 1 denotate Bi 2>&1};
$output = qx{../src/task rc:$rc rrr 2>&1};
unlike ($output, qr/Bert.+\d{1,2}\/\d{1,2}\/\d{4} Bibo/ms,                  "$ut: Delete partial match");
like ($output, qr/Bert.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,         "$ut: Kermit the frog now second annotation");

if ($^O =~ /cygwin/)
{
  skip ("$ut: Denotate is case sensitive");
  skip ("$ut: Kermit the frog still second annoation");
}
else
{
  qx{../src/task rc:$rc 1 denotate BErt 2>&1};
  $output = qx{../src/task rc:$rc rrr 2>&1};
  like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} Bert/ms,                   "$ut: Denotate is case sensitive");
  like ($output, qr/Bert.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,       "$ut: Kermit the frog still second annoation"); # 15
}

qx{../src/task rc:$rc 1 denotate Kermit 2>&1};
$output = qx{../src/task rc:$rc rrr 2>&1};
like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} Bert/ms,                     "$ut: Exact match deletion - Bert");
like ($output, qr/Bert.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,         "$ut: Exact match deletion - Kermit the frog");
like ($output, qr/frog.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,         "$ut: Exact match deletion - Kermit the frog");
like ($output, qr/frog.+\d{1,2}\/\d{1,2}\/\d{4} Kermit and Miss Piggy/ms,   "$ut: Exact match deletion - Kermit and Miss Piggy");

qx{../src/task rc:$rc 1 denotate Kermit the 2>&1};
$output = qx{../src/task rc:$rc rrr 2>&1};
like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} Bert/ms,                     "$ut: Delete just one annotation - Bert"); # 20
like ($output, qr/Bert.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,         "$ut: Delete just one annotation - Kermit the frog");
like ($output, qr/frog.+\d{1,2}\/\d{1,2}\/\d{4} Kermit and Miss Piggy/ms,   "$ut: Delete just one annotation - Kermit and Miss Piggy");

qx{../src/task rc:$rc 1 denotate Kermit a 2>&1};
$output = qx{../src/task rc:$rc rrr 2>&1};
like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} Bert/ms,                     "$ut: Delete partial match - Bert");
like ($output, qr/Bert.+\d{1,2}\/\d{1,2}\/\d{4} Kermit the frog/ms,         "$ut: Delete partial match - Kermit the frog");
unlike ($output, qr/frog.+\d{1,2}\/\d{1,2}\/\d{4} Kermit and Miss Piggy/ms, "$ut: Delete partial match - Kermit and Miss Piggy"); # 25

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
