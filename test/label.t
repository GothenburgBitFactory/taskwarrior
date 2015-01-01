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
use Test::More tests => 5;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'custom.rc')
{
  print $fh "data.location=.\n",
            "report.foo.description=DESC\n",
            "report.foo.labels=ID,DESCRIPTION\n",
            "report.foo.columns=id,description\n",
            "report.foo.sort=id+\n",
            "report.foo.filter=project:A\n";
  close $fh;
}

# Generate the help screen, and locate the custom report on it.
my $output = qx{../src/task rc:custom.rc help 2>&1};
like ($output, qr/task <filter> foo\s+DESC\n/m, 'report.foo');

qx{../src/task rc:custom.rc add project:A one 2>&1};
qx{../src/task rc:custom.rc add two 2>&1};
$output = qx{../src/task rc:custom.rc foo 2>&1};
like ($output,   qr/ID/,          'custom label for id column');
like ($output,   qr/DESCRIPTION/, 'custom label for description column');
like ($output,   qr/one/,         'custom filter included');
unlike ($output, qr/two/,         'custom filter excluded');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data  custom.rc);
exit 0;

