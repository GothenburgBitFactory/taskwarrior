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
use Test::More tests => 3;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

# Bug 697: Making a blocking task recur breaks dependency.
#   1. Create 2 tasks: "foo" and "bar".
#   2. Give "bar" a due date.
#   3. Make "foo" depend on "bar".
#   4. Make "bar" recur yearly.
qx{../src/task rc:bug.rc add foo 2>&1};
qx{../src/task rc:bug.rc add bar 2>&1};
qx{../src/task rc:bug.rc 2 modify due:eom 2>&1};
qx{../src/task rc:bug.rc 1 modify depends:2 2>&1};
qx{../src/task rc:bug.rc 2 modify recur:yearly 2>&1};
qx{../src/task rc:bug.rc ls}; # Generate child recurring tasks.

# Here is the pending.data:
#
# [depends:"0e200da3-7d73-44e8-9a51-6b0263414db6" description:"foo" entry:"1330839054" status:"pending" uuid:"dcd04ff6-c074-47d3-9346-211a1b739bdd"]
# [description:"bar" due:"1333166400" entry:"1330839054" mask:"-" recur:"yearly" status:"recurring" uuid:"0e200da3-7d73-44e8-9a51-6b0263414db6"]
# [description:"bar" due:"1333166400" entry:"1330839054" imask:"0" parent:"0e200da3-7d73-44e8-9a51-6b0263414db6" recur:"yearly" status:"pending" uuid:"538daa82-d481-4033-b98e-06e98fbe1073"]
#
# The problem is that although 1 --> 2, 2 is now a recurring parent, and as 1
# depends on the parent's UUID, it is not something transferred to the child on
# generation, because the dependency belongs with 1, not 2.

my $output = qx{../src/task rc:bug.rc 1 info 2>&1};
like ($output, qr/is blocked by\s+2/, 'verified 1 --> 2');

$output = qx{../src/task rc:bug.rc 2 info 2>&1};
like ($output, qr/is blocking\s+1/, 'verified 2 <-- 1');

$output = qx{../src/task rc:bug.rc 3 info 2>&1};
like ($output, qr/is blocked by\s+2/, 'verified 1 --> 2');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
exit 0;

