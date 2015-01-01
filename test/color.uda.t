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

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'color.rc')
{
  print $fh "data.location=.\n",
            "color.uda.x=red\n",
            "uda.x.type=numeric\n",
            "uda.x.label=X\n",
            "uda.y.type=numeric\n",
            "uda.y.label=Y\n",
            "color.uda.y.4=blue\n",
            "color.alternate=\n",
            "_forcecolor=1\n";
  close $fh;
}

qx{../src/task rc:color.rc add one 2>&1};
qx{../src/task rc:color.rc add two x:3 y:2 2>&1};
qx{../src/task rc:color.rc add three y:4 2>&1};
my $output = qx{../src/task rc:color.rc list 2>/dev/null};

unlike ($output, qr/ \033\[32m .* one .* \033\[0m /x, 'No color.uda');
like   ($output, qr/ \033\[31m .* two .* \033\[0m /x, 'Found color.uda');
like   ($output, qr/ \033\[34m .* three .* \033\[0m /x, 'Found color.uda.x');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data color.rc);
exit 0;

