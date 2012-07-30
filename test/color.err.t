#! /usr/bin/env perl
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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', 'color.rc')
{
  print $fh "data.location=.\n",
            "color.header=blue\n",
            "color.footnote=red\n",
            "color.error=yellow\n",
            "color.debug=green\n",
            "_forcecolor=1\n";
  close $fh;
  ok (-r 'color.rc', 'Created color.rc');
}

# Test the errors colors
my $output = qx{../src/task rc:color.rc rc.debug:on add due:__ 2>&1 >/dev/null};
like ($output, qr/^\033\[33mThe\ duration\ '__'\ was\ not\ recognized\ as\ valid,\ with\ correct\ units\ like\ '3days'\.\033\[0m$/xms, 'color.error');
like ($output, qr/^\033\[32mTimer\ Config::load\ \(color.rc\) .* \033\[0m$/xms, 'color.debug');
like ($output, qr/^\033\[34mUsing\ alternate\ .taskrc\ file\ color.rc\033\[0m$/xms, 'color.header');
like ($output, qr/^\033\[31mConfiguration\ override\ rc.debug:on\033\[0m$/xms, 'color.footnote');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key color.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'color.rc', 'Cleanup');

exit 0;

