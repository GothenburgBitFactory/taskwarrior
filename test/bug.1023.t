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
use Test::More tests => 9;

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
            "default.project=home\n";
  close $fh;
}

# Bug 1023: rc.default.project gets applied during modify, and should not.
qx{../src/task rc:$rc add foo project:garden 2>&1};
qx{../src/task rc:$rc add bar 2>&1};
qx{../src/task rc:$rc add baz rc.default.project= 2>&1};

my $output = qx{../src/task rc:$rc 1 info 2>&1};
like ($output, qr/Project\s*garden/,     "$ut: default project not applied when otherwise specified.");

$output = qx{../src/task rc:$rc 2 info 2>&1};
like ($output, qr/Project\s*home/,       "$ut: default project applied when blank.");

$output = qx{../src/task rc:$rc 3 info 2>&1};
like ($output, qr/^Description\s+baz$/m, "$ut: task baz shown.");
unlike ($output, qr/Project\s*home/,     "$ut: no project applied when default project is blank.");

$output = qx{../src/task rc:$rc 3 modify +tag 2>&1};
like ($output, qr/^Modified 1 task.$/m,  "$ut: task modified.");
unlike ($output, qr/Project\s*home/,     "$ut: default project not applied on modification.");

qx{../src/task rc:$rc 1 modify project: 2>&1};
$output = qx{../src/task rc:$rc 1 info 2>&1};
like ($output, qr/^Description\s+foo$/m, "$ut: task foo shown.");
unlike ($output, qr/Project\s*garden/,   "$ut: default project not re-applied on attribute removal.");
unlike ($output, qr/Project\s*home/,     "$ut: default project not re-applied on attribute removal.");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

