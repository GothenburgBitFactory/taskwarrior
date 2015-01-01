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
use Test::More tests => 8;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'ids.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
}

# Test the count command.
qx{../src/task rc:ids.rc add one    +A +B 2>&1};
qx{../src/task rc:ids.rc add two    +A    2>&1};
qx{../src/task rc:ids.rc add three  +A +B 2>&1};
qx{../src/task rc:ids.rc add four         2>&1};
qx{../src/task rc:ids.rc add five   +A +B 2>&1};

my $output = qx{../src/task rc:ids.rc ids +A 2>/dev/null};
like ($output, qr/^1-3,5$/, 'ids +A --> 1-3,5');

$output = qx{../src/task rc:ids.rc ids +B 2>/dev/null};
like ($output, qr/^1,3,5$/, 'ids +B --> 1,3,5');

$output = qx{../src/task rc:ids.rc ids +A -B 2>/dev/null};
like ($output, qr/^2$/, 'ids +A -B --> 2');

$output = qx{../src/task rc:ids.rc _ids +A};
like ($output, qr/^1\n2\n3\n5$/, '_ids +A --> 1\n2\n3\n5');

$output = qx{../src/task rc:ids.rc _zshids +A};
like ($output, qr/^1:one\n2:two\n3:three\n5:five$/, '_zshids +A --> 1:one\n2:two\n3:three\n5:five');

$output = qx{../src/task rc:ids.rc uuids +A 2>/dev/null};
like ($output, qr/^[0-9a-f-]+,[0-9a-f-]+,[0-9a-f-]+,[0-9a-f-]+$/, 'uuids +A --> uuid,uuid,uuid,uuid');

$output = qx{../src/task rc:ids.rc _uuids +A};
like ($output, qr/^[0-9a-f-]+\n[0-9a-f-]+\n[0-9a-f-]+\n[0-9a-f-]+$/, '_uuids +A --> uuid\nuuid\nuuid\nuuid');

# The order of the task may not be respected with _zshuuids
$output = qx{../src/task rc:ids.rc _zshuuids +A};
like ($output, qr/^[0-9a-f-]+:[a-z]+\n[0-9a-f-]+:[a-z]+\n[0-9a-f-]+:[a-z]+\n[0-9a-f-]+:[a-z]+$/, '_zshuuids +A --> uuid:*\nuuid:*\nuuid:*\nuuid:*');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data ids.rc);
exit 0;

