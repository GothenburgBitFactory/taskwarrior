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
use Test::More tests => 6;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'uda.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "uda.extra.type=string\n",
            "uda.extra.label=Extra\n";
  close $fh;
}

# Add a task with a defined UDA.
qx{../src/task rc:uda.rc add one extra:foo 2>&1};
my $output = qx{../src/task rc:uda.rc 1 info 2>&1};
like ($output, qr/Extra\s+foo/, 'UDA created');

# Eliminate the UDA.
if (open my $fh, '>', 'uda.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
}

# Observe the UDA properly reported by the 'info' command.
$output = qx{../src/task rc:uda.rc 1 info 2>&1};
like ($output, qr/extra\s+foo/, 'UDA orphan shown');

# Modify the task, ensure UDA preserved.
qx{../src/task rc:uda.rc 1 modify /one/two/ 2>&1};
$output = qx{../src/task rc:uda.rc 1 info 2>&1};
like ($output, qr/extra\s+foo/, 'UDA orphan preserved by modification');

# Make sure an orphan UDA is exported.
$output = qx{../src/task rc:uda.rc export 2>&1};
like ($output, qr/"extra":"foo"/, 'UDA orphan exported');

# Make sure an orphan UDA can be exported.
if (open my $fh, '>', 'import.txt')
{
  print $fh <<EOF;
{"uuid":"00000000-0000-0000-0000-000000000000","description":"two","status":"pending","entry":"1234567889","extra":"bar"}
EOF

  close $fh;
}

$output = qx{../src/task rc:uda.rc import import.txt 2>&1 >/dev/null};
like ($output, qr/Imported 1 tasks\./, 'UDA orphan import');
$output = qx{../src/task rc:uda.rc 2 info 2>&1};
like ($output, qr/extra\s+bar/, 'UDA orphan imported and visible');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data uda.rc import.txt);
exit 0;

