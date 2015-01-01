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
use Test::More tests => 4;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "report.test.columns=id,project\n";
  close $fh;
}

# Feature: variable to control printing of empty columns
qx{../src/task rc:bug.rc add sample desc 2>&1};
qx{../src/task rc:bug.rc add withP project:house 2>&1};

my $output = qx{../src/task test sample rc:bug.rc 2>&1};
unlike ($output, qr/Project/, 'empty \'project\' column is not printed by default');

$output = qx{../src/task test sample rc.print.empty.columns:yes rc:bug.rc 2>&1};
like ($output, qr/Project/, 'empty \'project\' column is printed if rc.print.empty.columns:yes');

$output = qx{../src/task test rc:bug.rc 2>&1};
like ($output, qr/Project/, 'non-empty \'project\' column is printed by default');

$output = qx{../src/task test rc.print.empty.columns:yes rc:bug.rc 2>&1};
like ($output, qr/Project/, 'non-empty \'project\' column is printed if rc.print.empty.columns:yes');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
exit 0;
