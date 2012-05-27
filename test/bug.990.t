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
use Test::More tests => 3;

# Create the rc file.
if (open my $fh, '>', 'color.rc')
{
  print $fh "data.location=.\n",
            "color.pri.L=green\n",
            "color.tagged=red\n",
            "_forcecolor=1\n";
  close $fh;
  ok (-r 'color.rc', 'Created color.rc');
}

# Bug that colored any task with both priority:L and a tag as though
# rc.color.tagged had a higher precedence than rc.color.pri.L, which it is not.

qx{../src/task rc:color.rc add test +test pri:L};
my $output = qx{../src/task rc:color.rc list};
like ($output, qr/ \033\[32m        .* test .* \033\[0m /x, 'Colored with the priority color, which has precedence over the tagged color');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key color.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'color.rc', 'Cleanup');

exit 0;

