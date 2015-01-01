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
if (open my $fh, '>', 'verbose.rc')
{
  print $fh "data.location=.\n",
            "print.empty.columns=yes\n";
  close $fh;
}

# Verbosity: 'new-id'
my $output = qx{../src/task rc:verbose.rc rc.verbose:new-id add Sample1 2>&1};
like ($output, qr/Created task \d/, '\'new-id\' verbosity good');

$output = qx{../src/task rc:verbose.rc rc.verbose:nothing add Sample2 2>&1};
unlike ($output, qr/Created task \d/, '\'new-id\' verbosity good');

# Verbosity: 'label'
$output = qx{../src/task rc:verbose.rc ls rc.verbose:label 2>&1};
like ($output, qr/ID.+A.+D.+Project.+Tags.+R.+Wait.+S.+Due.+Until.+Description/, '\'label\' verbosity good');

# Verbosity: 'affected'
$output = qx{../src/task rc:verbose.rc ls rc.verbose:affected 2>&1};
like ($output, qr/^\d+ tasks$/ms, '\'affected\' verbosity good');

# Off
$output = qx{../src/task rc:verbose.rc ls rc.verbose:nothing 2>&1};
unlike ($output, qr/^\d+ tasks$/ms, '\'affected\' verbosity good');
unlike ($output, qr/ID.+Project.+Pri.+Description/, '\'label\' verbosity good');

# TODO Verbosity: 'blank'
# TODO Verbosity: 'header'
# TODO Verbosity: 'edit'
# TODO Verbosity: 'special'
# TODO Verbosity: 'project'

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data verbose.rc);
exit 0;

