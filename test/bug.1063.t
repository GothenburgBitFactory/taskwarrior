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
use Test::More tests => 6;

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
            "confirmation=off\n",
            "uda.foo.type=numeric\n",
            "uda.foo.label=Foo\n",
            "report.bar.columns=foo,description\n",
            "report.bar.description=Bar\n",
            "report.bar.labels=Foo,Desc\n",
            "report.bar.sort=foo-\n";
  close $fh;
}

# Bug 1063 - Numeric UDA fields are not sortable.
qx{../src/task rc:$rc add four  foo:4 2>&1};
ok ($? == 0, "$ut: add four");
qx{../src/task rc:$rc add one   foo:1 2>&1};
ok ($? == 0, "$ut: add one");
qx{../src/task rc:$rc add three foo:3 2>&1};
ok ($? == 0, "$ut: add three");
qx{../src/task rc:$rc add two   foo:2 2>&1};
ok ($? == 0, "$ut: add two");

my $output = qx{../src/task rc:$rc bar 2>&1};
like ($output, qr/4.+3.+2.+1/ms, "$ut: Default descending sort correct");

$output = qx{../src/task rc:$rc bar rc.report.bar.sort=foo+ 2>&1};
like ($output, qr/1.+2.+3.+4/ms, "$ut: Default ascending sort correct");

## Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
