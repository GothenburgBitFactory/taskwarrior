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
use Test::More tests => 36;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Feature 891: UUID filter should be uuid.endswith by default
qx{../src/task rc:bug.rc add one};
qx{../src/task rc:bug.rc add two};
my $output = qx{../src/task rc:bug.rc 1 info};
my ($uuid) = $output =~ /UUID\s+(\S{36})/ms;

$output = qx{../src/task rc:bug.rc $uuid list rc.debug:1};
like ($output, qr/one/, "Found with $uuid");

my ($short) = $uuid =~ /^(.{35})/;
$output = qx{../src/task rc:bug.rc $short list rc.debug:1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{34})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{33})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{32})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{31})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{30})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{29})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{28})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{27})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{26})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{25})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{24})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{23})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{22})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{21})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{20})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{19})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{18})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{17})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{16})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{15})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{14})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{13})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{12})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{11})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{10})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{9})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{8})/;
$output = qx{../src/task rc:bug.rc $short list};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{7})/;
$output = qx{../src/task rc:bug.rc $short list};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{6})/;
$output = qx{../src/task rc:bug.rc $short list};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{5})/;
$output = qx{../src/task rc:bug.rc $short list};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{4})/;
$output = qx{../src/task rc:bug.rc $short list};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{3})/;
$output = qx{../src/task rc:bug.rc $short list};
unlike ($output, qr/one/, "Not found with $short");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

