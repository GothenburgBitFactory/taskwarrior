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
if (open my $fh, '>', 'enp.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

# Test the en passant feature.
qx{../src/task rc:enp.rc add foo 2>&1};
qx{../src/task rc:enp.rc add foo bar 2>&1};
qx{../src/task rc:enp.rc 1,2 done /foo/FOO/ pri:H +tag 2>&1};
my $output = qx{../src/task rc:enp.rc info 1 2>&1};
like ($output, qr/Status\s+Completed/,    'en passant 1 status change');
like ($output, qr/Description\s+FOO/,     'en passant 1 description change');
like ($output, qr/Priority\s+H/,          'en passant 1 description change');
like ($output, qr/Tags\s+tag/,            'en passant 1 description change');
$output = qx{../src/task rc:enp.rc info 2 2>&1};
like ($output, qr/Status\s+Completed/,    'en passant 2 status change');
like ($output, qr/Description\s+FOO bar/, 'en passant 2 description change');
like ($output, qr/Priority\s+H/,          'en passant 2 description change');
like ($output, qr/Tags\s+tag/,            'en passant 2 description change');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data enp.rc);
exit 0;

