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
use Test::More tests => 18;

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
            "report.foo.description=Sample\n",
            "report.foo.columns=id,due,description\n",
            "report.foo.labels=ID,Due,Description\n",
            "report.foo.sort=due+\n",
            "report.foo.filter=status:pending\n",
            "report.foo.dateformat=MD\n";
  close $fh;
}

# Bug #418: due.before:eow not working
#   - with dateformat 'MD'
qx{../src/task rc:$rc add one   due:6/28/2010 2>&1};
qx{../src/task rc:$rc add two   due:6/29/2010 2>&1};
qx{../src/task rc:$rc add three due:6/30/2010 2>&1};
qx{../src/task rc:$rc add four  due:7/1/2010 2>&1};
qx{../src/task rc:$rc add five  due:7/2/2010 2>&1};
qx{../src/task rc:$rc add six   due:7/3/2010 2>&1};
qx{../src/task rc:$rc add seven due:7/4/2010 2>&1};
qx{../src/task rc:$rc add eight due:7/5/2010 2>&1};
qx{../src/task rc:$rc add nine  due:7/6/2010 2>&1};

my $output = qx{../src/task rc:$rc foo 2>&1};
like ($output, qr/one/ms,     "$ut: task 1 listed");
like ($output, qr/two/ms,     "$ut: task 2 listed");
like ($output, qr/three/ms,   "$ut: task 3 listed");
like ($output, qr/four/ms,    "$ut: task 4 listed");
like ($output, qr/five/ms,    "$ut: task 5 listed");
like ($output, qr/six/ms,     "$ut: task 6 listed");
like ($output, qr/seven/ms,   "$ut: task 7 listed");
like ($output, qr/eight/ms,   "$ut: task 8 listed");
like ($output, qr/nine/ms,    "$ut: task 9 listed");

$output = qx{../src/task rc:$rc foo due.before:7/2/2010 2>&1};
like ($output, qr/one/ms,     "$ut: task 1 listed");
like ($output, qr/two/ms,     "$ut: task 2 listed");
like ($output, qr/three/ms,   "$ut: task 3 listed");
like ($output, qr/four/ms,    "$ut: task 4 listed");
unlike ($output, qr/five/ms,  "$ut: task 5 not listed");
unlike ($output, qr/six/ms,   "$ut: task 6 not listed");
unlike ($output, qr/seven/ms, "$ut: task 7 not listed");
unlike ($output, qr/eight/ms, "$ut: task 8 not listed");
unlike ($output, qr/nine/ms,  "$ut: task 9 not listed");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

