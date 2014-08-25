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
use Test::More tests => 1;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "shadow.file=shadow.txt\n",
            "shadow.command=list\n";
  close $fh;
}

=pod
I found a bug in the current version of task. Using recur and a shadow file will
lead to an infinite loop. To reproduce it, define a shadow file in the .taskrc,
set a command for it that rebuilds the database, e.g. "list", and then add a
task with a recurrence set, e.g. "task add due:today recur:1d infinite loop".
Task will then loop forever and add the same recurring task until it runs out of
memory. So I checked the source and I believe I found the cause.
handleRecurrence() in task.cpp will modify the mask, but writes it only after it
has added all new tasks. Adding the task will, however, invoke onChangeCallback,
which starts the same process all over again.
=cut

eval
{
  $SIG{'ALRM'} = sub {die "alarm\n"};
  alarm 10;
  my $output = qx{../src/task rc:$rc list 2>&1;
                  ../src/task rc:$rc add due:today recur:1d infinite loop 2>&1;
                  ../src/task rc:$rc info 1 2>&1};
  alarm 0;

  like ($output, qr/^Description\s+infinite loop\n/m, "$ut: no hang");
};

if ($@ eq "alarm\n")
{
  fail ("$ut: task hang on add or recurring task, with shadow file, for 10s");
}

# Cleanup.
unlink qw(shadow.txt pending.data completed.data undo.data backlog.data), $rc;
exit 0;

