#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 13;

# Create the rc file.
if (open my $fh, '>', 'annual.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'annual.rc', 'Created annual.rc');
}

# If a task is added with a due date ten years ago, with an annual recurrence,
# then the synthetic tasks in between then and now have a due date that creeps.
#
# ID Project Pri Due        Active Age Description
# -- ------- --- ---------- ------ --- -----------
#  2               1/1/2000          - foo
#  3             12/31/2000          - foo
#  4             12/31/2001          - foo
#  5             12/31/2002          - foo
#  6             12/31/2003          - foo
#  7             12/30/2004          - foo
#  8             12/30/2005          - foo
#  9             12/30/2006          - foo
# 10             12/30/2007          - foo
# 11             13/29/2008          - foo
# 12             12/29/2009          - foo

qx{../src/task rc:annual.rc add foo due:1/1/2000 recur:annual until:1/1/2009};
my $output = qx{../src/task rc:annual.rc list};
like ($output, qr/2\s+1\/1\/2000\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 2 no creep');
like ($output, qr/3\s+1\/1\/2001\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 3 no creep');
like ($output, qr/4\s+1\/1\/2002\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 4 no creep');
like ($output, qr/5\s+1\/1\/2003\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 5 no creep');
like ($output, qr/6\s+1\/1\/2004\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 6 no creep');
like ($output, qr/7\s+1\/1\/2005\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 7 no creep');
like ($output, qr/8\s+1\/1\/2006\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 8 no creep');
like ($output, qr/9\s+1\/1\/2007\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 9 no creep');
like ($output, qr/10\s+1\/1\/2008\s+(?:-|\d+\ssecs?)\s+foo/, 'synthetic 10 no creep');
like ($output, qr/11\s+1\/1\/2009\s+(?:-|\d+\ssecs?)\s+foo/, 'synthetic 11 no creep');

$output = qx{../src/task rc:annual.rc diag};
like ($output, qr/No duplicates found/, 'No duplicate UUIDs detected');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key annual.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'annual.rc', 'Cleanup');

exit 0;

