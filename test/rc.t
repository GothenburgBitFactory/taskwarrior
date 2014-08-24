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
use File::Path;
use Test::More tests => 13;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file, using rc.name:value.
unlink $rc;
rmtree 'foo', 0, 0;
qx{echo 'y'|../src/task rc:$rc rc.data.location:foo _version 2>&1};

ok (-r $rc, "$ut: Created default rc file");
ok (-d 'foo', "$ut: Created default data directory");

rmtree 'foo', 0, 0;
ok (!-r 'foo', "$ut: Removed foo");

unlink $rc;
ok (!-r $rc, "$ut: Removed $rc");

# Do it all again, with rc.name=value.
qx{echo 'y'|../src/task rc:$rc rc.data.location:foo _version 2>&1};

ok (-r $rc, "$ut: Created default rc file");
ok (-d 'foo', "$ut: Created default data directory");

# Add a setting.
qx{echo 'y'|../src/task rc:$rc config must_be_unique old 2>&1};
my $output = qx{../src/task rc:$rc show 2>&1};
like ($output, qr/^must_be_unique\s+old/ms, "$ut: config setting a new value");

qx{echo 'y'|../src/task rc:$rc config must_be_unique new 2>&1};
$output = qx{../src/task rc:$rc show 2>&1};
like ($output, qr/^must_be_unique\s+new/ms, "$ut: config overwriting an existing value");

qx{echo 'y'|../src/task rc:$rc config must_be_unique '' 2>&1};
$output = qx{../src/task rc:$rc show 2>&1};
like ($output, qr/^must_be_unique/ms, "$ut: config setting a blank value");

qx{echo 'y'|../src/task rc:$rc config must_be_unique 2>&1};
$output = qx{../src/task rc:$rc show 2>&1};
unlike ($output, qr/^must_be_unique/ms, "$ut: config removing a value");

# 'report.:b' is designed to get past the config command checks for recognized
# names.
qx{echo 'y'|../src/task rc:$rc config -- report.:b +c 2>&1};
$output = qx{../src/task rc:$rc show 2>&1};
like ($output, qr/^report\.:b\s+\+c/ms, "$ut: the -- operator is working");

# Make sure the value is accepted if it has multiple words.
qx{echo 'y'|../src/task rc:$rc config must_be_unique 'one two three' 2>&1};
$output = qx{../src/task rc:$rc show 2>&1};
like ($output, qr/^must_be_unique\s+one two three/ms, "$ut: config allows multi-word quoted values");

qx{echo 'y'|../src/task rc:$rc config must_be_unique one two three 2>&1};
$output = qx{../src/task rc:$rc show 2>&1};
like ($output, qr/^must_be_unique\s+one two three/ms, "$ut: config allows multi-word unquoted values");

rmtree 'foo', 0, 0;
unlink $rc;
exit 0;

