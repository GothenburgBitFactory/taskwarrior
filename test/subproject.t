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
use Test::More tests => 8;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'sp.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

my $setup = "../src/task rc:sp.rc add project:abc abc 2>&1;"
          . "../src/task rc:sp.rc add project:ab  ab 2>&1;"
          . "../src/task rc:sp.rc add project:a   a 2>&1;"
          . "../src/task rc:sp.rc add project:b   b 2>&1;";
qx{$setup};

my $output = qx{../src/task rc:sp.rc list project:b 2>&1};
like ($output, qr/\bb\s/m, 'abc,ab,a,b | b -> b');

$output = qx{../src/task rc:sp.rc list project:a 2>&1};
like ($output, qr/\babc\s/m, 'abc,ab,a,b | a -> abc');
like ($output, qr/\bab\s/m,  'abc,ab,a,b | a -> ab');
like ($output, qr/\ba\s/m,   'abc,ab,a,b | a -> a');

$output = qx{../src/task rc:sp.rc list project:ab 2>&1};
like ($output, qr/\babc\s/m, 'abc,ab,a,b | a -> abc');
like ($output, qr/\bab\s/m,  'abc,ab,a,b | a -> ab');

$output = qx{../src/task rc:sp.rc list project:abc 2>&1};
like ($output, qr/\babc\s/m, 'abc,ab,a,b | a -> abc');

$output = qx{../src/task rc:sp.rc list project:abcd 2>&1 >/dev/null};
like ($output, qr/No matches\./, 'abc,ab,a,b | abcd -> nul');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data sp.rc);
exit 0;

