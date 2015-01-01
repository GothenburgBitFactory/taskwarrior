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
use Test::More tests => 10;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n";
  close $fh;
}

# Test the duplicate command.
qx{../src/task rc:$rc add foo 2>&1};
qx{../src/task rc:$rc 1 duplicate 2>&1};
my $output = qx{../src/task rc:$rc info 2 2>&1};
like ($output, qr/ID\s+2/,            "$ut: duplicate new id");
like ($output, qr/Status\s+Pending/,  "$ut: duplicate same status");
like ($output, qr/Description\s+foo/, "$ut: duplicate same description");

# Test the en passant modification while duplicating.
qx{../src/task rc:$rc 1 duplicate priority:H /foo/FOO/ +tag 2>&1};
$output = qx{../src/task rc:$rc info 3 2>&1};
like ($output, qr/ID\s+3/,            "$ut: duplicate new id");
like ($output, qr/Status\s+Pending/,  "$ut: duplicate same status");
like ($output, qr/Description\s+FOO/, "$ut: duplicate modified description");
like ($output, qr/Priority\s+H/,      "$ut: duplicate added priority");
like ($output, qr/Tags\s+tag/,        "$ut: duplicate added tag");

# Test the output of the duplicate command - returning id of duplicated task
$output = qx{../src/task rc:$rc 1 duplicate 2>&1};
like ($output, qr/Duplicated\stask\s1\s'foo'/, "$ut: duplicate output task id and description");
like ($output, qr/Created\s+task\s+4/,         "$ut: duplicate output of new task id");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

