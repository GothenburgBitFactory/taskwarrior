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
use Test::More tests => 4;

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
            "verbose=affected\n";
  close $fh;
}

# Bug with completion of "des" in task descriptions and annotations. It happens
# for all the shortcuts for column attributes that are automatically completed.
# This is because DOM elements are checked before standard words when strings
# are tokenized.

# Check that the completion is inactive in task descriptions
qx{../src/task rc:$rc add des 2>&1};
qx{../src/task rc:$rc 1 annotate des 2>&1};
my $output = qx{../src/task rc:$rc 1 info 2>&1};
like ($output, qr/^Description\s+des$/ms, "$ut: Attribute not completed in description");
unlike ($output, qr/description/ms,       "$ut: Attribute not completed in description");

# Check that the completion works when needed
$output = qx{../src/task rc:$rc des:des 2>&1};
like ($output, qr/^1 task$/ms,            "$ut: Task found using its description");

qx{../src/task rc:$rc add entrée interdite 2>&1};
$output = qx{../src/task rc:$rc list interdite 2>&1};
like ($output, qr/entrée interdite/,      "$ut: 'entrée' left intact");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
