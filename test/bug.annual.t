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
use Test::More tests => 11;

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
            "dateformat=m/d/Y\n",
            "report.annual.labels=ID,Due,Description\n",
            "report.annual.columns=id,due,description\n",
            "report.annual.filter=status:pending\n",
            "report.annual.sort=due+\n";
  close $fh;
}

# If a task is added with a due date ten years ago, with an annual recurrence,
# then the synthetic tasks in between then and now have a due date that creeps.
#
# ID Due        Description
# -- ---------- -----------
#  4 1/1/2002   foo
#  5 1/1/2003   foo
#  6 1/1/2004   foo
#  7 1/1/2005   foo
#  8 1/1/2006   foo
#  9 1/1/2007   foo
# 10 1/1/2008   foo
# 11 1/1/2009   foo
# 12 1/1/2010   foo
#  2 1/1/2000   foo
#  3 1/1/2001   foo

qx{../src/task rc:$rc add foo due:1/1/2000 recur:annual until:1/1/2009 2>&1};
my $output = qx{../src/task rc:$rc annual 2>&1};
like ($output, qr/2\s+1\/1\/2000\s+foo/,  "$ut: synthetic 2 no creep");
like ($output, qr/3\s+1\/1\/2001\s+foo/,  "$ut: synthetic 3 no creep");
like ($output, qr/4\s+1\/1\/2002\s+foo/,  "$ut: synthetic 4 no creep");
like ($output, qr/5\s+1\/1\/2003\s+foo/,  "$ut: synthetic 5 no creep");
like ($output, qr/6\s+1\/1\/2004\s+foo/,  "$ut: synthetic 6 no creep");
like ($output, qr/7\s+1\/1\/2005\s+foo/,  "$ut: synthetic 7 no creep");
like ($output, qr/8\s+1\/1\/2006\s+foo/,  "$ut: synthetic 8 no creep");
like ($output, qr/9\s+1\/1\/2007\s+foo/,  "$ut: synthetic 9 no creep");
like ($output, qr/10\s+1\/1\/2008\s+foo/, "$ut: synthetic 10 no creep");
like ($output, qr/11\s+1\/1\/2009\s+foo/, "$ut: synthetic 11 no creep");

$output = qx{../src/task rc:$rc diag 2>&1};
like ($output, qr/No duplicates found/,   "$ut: No duplicate UUIDs detected");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

