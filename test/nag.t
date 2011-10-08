#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 13;

# Create the rc file.
if (open my $fh, '>', 'nag.rc')
{
  print $fh "data.location=.\n",
            "nag=NAG\n";
  close $fh;
  ok (-r 'nag.rc', 'Created nag.rc');
}

my $setup = "../src/task rc:nag.rc add due:yesterday one;"
          . "../src/task rc:nag.rc add due:tomorrow two;"
          . "../src/task rc:nag.rc add priority:H three;"
          . "../src/task rc:nag.rc add priority:M four;"
          . "../src/task rc:nag.rc add priority:L five;"
          . "../src/task rc:nag.rc add six;";
qx{$setup};

like   (qx{../src/task rc:nag.rc 6 do}, qr/NAG/, 'do pri: -> nag');
like   (qx{../src/task rc:nag.rc 5 do}, qr/NAG/, 'do pri:L -> nag');
like   (qx{../src/task rc:nag.rc 4 do}, qr/NAG/, 'do pri:M-> nag');
like   (qx{../src/task rc:nag.rc 3 do}, qr/NAG/, 'do pri:H-> nag');
like   (qx{../src/task rc:nag.rc 2 do}, qr/NAG/, 'do due:tomorrow -> nag');
unlike (qx{../src/task rc:nag.rc 1 do}, qr/NAG/, 'do due:yesterday -> no nag');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink 'nag.rc';
ok (!-r 'nag.rc', 'Removed nag.rc');

exit 0;

