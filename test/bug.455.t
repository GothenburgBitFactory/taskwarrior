#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

# Create the rc file.
if (open my $fh, '>', '455.rc')
{
  print $fh "data.location=.\n";
  print $fh "print.empty.columns=no\n";

  close $fh;
  ok (-r '455.rc', 'Created 455.rc');
}

# Bug #455 - Text alignment in reports is broken when text contains wide utf8
#            characters

qx{../src/task rc:455.rc add abc pro:Bar\x{263A} 2>&1};
qx{../src/task rc:455.rc add def pro:Foo! 2>&1};

my $output = qx{../src/task rc:455.rc ls 2>&1};

# Project + ' ' == 4
like ($output, qr/\S\s{4}abc/ms, 'bug 455 - correct spacing in utf8 task');
like ($output, qr/\S\s{4}def/ms, 'bug 455 - correct spacing in non utf8 task');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data 455.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r '455.rc', 'Cleanup');

exit 0;
