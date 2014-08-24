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
use Test::More tests => 5;

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

# Bug #414: Tags filtering not working with unicode characters

# Add a task with a UTF-8 tag.
qx{../src/task rc:$rc add one +osobní 2>&1};
my $output = qx{../src/task rc:$rc ls +osobní 2>&1};
like ($output, qr/one/,   "$ut: found UTF8 tag osobní");

$output = qx{../src/task rc:$rc ls -osobní 2>&1};
like ($output, qr/^No matches.$/m, "$ut: not found UTF8 tag osobní");

# And a different one
qx{../src/task rc:$rc add two +föo 2>&1};
$output = qx{../src/task rc:$rc ls +föo 2>&1};
like ($output, qr/two/,   "$ut: found UTF8 tag föo");

$output = qx{../src/task rc:$rc ls -föo 2>&1};
like   ($output, qr/one/, "$ut: found UTF8 tag osobní");
unlike ($output, qr/two/, "$ut: not found UTF8 tag föo");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

