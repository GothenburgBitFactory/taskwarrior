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
use Test::More tests => 4;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'color.rc')
{
  print $fh "data.location=.\n",
            "search.case.sensitive=yes\n",
            "color=on\n",
            "color.alternate=\n",
            "color.keyword.red=red\n",
            "color.keyword.green=green\n",
            "color.keyword.yellow=yellow\n",
            "_forcecolor=1\n";
  close $fh;
}

# Test the add command.
qx{../src/task rc:color.rc add nothing 2>&1};
qx{../src/task rc:color.rc add red 2>&1};
qx{../src/task rc:color.rc add green 2>&1};
qx{../src/task rc:color.rc add -- annotation 2>&1};
qx{../src/task rc:color.rc 4 annotate yellow 2>&1};
my $output = qx{../src/task rc:color.rc list 2>&1};

like ($output, qr/ (?!<\033\[\d\dm) .* nothing    .* (?!>\033\[0m) /x, 'none');
like ($output, qr/ \033\[31m        .* red        .* \033\[0m      /x, 'color.keyword.red');
like ($output, qr/ \033\[32m        .* green      .* \033\[0m      /x, 'color.keyword.green');
like ($output, qr/ \033\[33m        .* annotation .* \033\[0m      /x, 'color.keyword.yellow (annotation)');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data color.rc);
exit 0;

