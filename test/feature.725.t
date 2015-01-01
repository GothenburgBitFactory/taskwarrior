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
if (open my $fh, '>', 'feature.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
}

# Feature 725: Feedback when tasks become unblocked.
qx{../src/task rc:feature.rc add one 2>&1};
qx{../src/task rc:feature.rc add two 2>&1};
qx{../src/task rc:feature.rc add three 2>&1};
qx{../src/task rc:feature.rc add four 2>&1};
qx{../src/task rc:feature.rc 1 modify depends:2,3 2>&1};
qx{../src/task rc:feature.rc 4 modify depends:1 2>&1};
my $output = qx{../src/task rc:feature.rc long 2>&1};
like ($output, qr/1.+2\s3/, 'Dependencies in place [1]');
like ($output, qr/4.+1/,    'Dependencies in place [4]');

# Trigger the feedback based on completion.
$output = qx{../src/task rc:feature.rc 2 done 2>&1};
unlike ($output, qr/Unblocked/, 'Completing first dependency does not trigger message');

$output = qx{../src/task rc:feature.rc 3 done 2>&1};
like ($output, qr/Unblocked/, 'Completing second dependency triggers message');

# Now trigger the feedback based on deletion.
$output = qx{../src/task rc:feature.rc long 2>&1};
like ($output, qr/2.+1/,    'Dependencies in place [2]');
$output = qx{../src/task rc:feature.rc 1 delete 2>&1};
like ($output, qr/Unblocked/, 'Deleting dependency triggers message');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data feature.rc);
exit 0;

