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
use Test::More tests => 32;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'pri.rc')
{
  print $fh "data.location=.\n",
            "verbose=off\n";
  close $fh;
}

# Verify that priorities can be select with the 'over' and 'under' modifiers.
qx{../src/task rc:pri.rc add H pri:H 2>&1};
qx{../src/task rc:pri.rc add M pri:M 2>&1};
qx{../src/task rc:pri.rc add L pri:L 2>&1};
qx{../src/task rc:pri.rc add _ 2>&1};

my $output = qx{../src/task rc:pri.rc ls priority.under:H 2>&1};
unlike ($output, qr/H/, 'pri H !< H');
  like ($output, qr/M/, 'pri M < H');
  like ($output, qr/L/, 'pri L < H');
  like ($output, qr/_/, 'pri _ < H');

$output = qx{../src/task rc:pri.rc ls priority.under:M 2>&1};
unlike ($output, qr/H/, 'pri H !< M');
unlike ($output, qr/M/, 'pri M !< M');
  like ($output, qr/L/, 'pri L < M');
  like ($output, qr/_/, 'pri _ < M');

$output = qx{../src/task rc:pri.rc ls priority.under:L 2>&1};
unlike ($output, qr/H/, 'pri H !< L');
unlike ($output, qr/M/, 'pri M !< L');
unlike ($output, qr/L/, 'pri L !< L');
  like ($output, qr/_/, 'pri _ < L');

$output = qx{../src/task rc:pri.rc ls priority.under: 2>&1};
unlike ($output, qr/H/, 'pri H !< _');
unlike ($output, qr/M/, 'pri M !< _');
unlike ($output, qr/L/, 'pri L !< _');
unlike ($output, qr/_/, 'pri _ !< _');

$output = qx{../src/task rc:pri.rc ls priority.over:H 2>&1};
unlike ($output, qr/H/, 'pri H !> H');
unlike ($output, qr/M/, 'pri M !> H');
unlike ($output, qr/L/, 'pri L !> H');
unlike ($output, qr/_/, 'pri _ !> H');

$output = qx{../src/task rc:pri.rc ls priority.over:M 2>&1};
  like ($output, qr/H/, 'pri H > M');
unlike ($output, qr/M/, 'pri M !> M');
unlike ($output, qr/L/, 'pri L !> M');
unlike ($output, qr/_/, 'pri _ !> M');

$output = qx{../src/task rc:pri.rc ls priority.over:L 2>&1};
  like ($output, qr/H/, 'pri H > L');
  like ($output, qr/M/, 'pri M > L');
unlike ($output, qr/L/, 'pri L !> L');
unlike ($output, qr/_/, 'pri _ !> L');

$output = qx{../src/task rc:pri.rc ls priority.over: 2>&1};
  like ($output, qr/H/, 'pri H > _');
  like ($output, qr/M/, 'pri M > _');
  like ($output, qr/L/, 'pri L > _');
unlike ($output, qr/_/, 'pri _ !> _');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data pri.rc);
exit 0;

