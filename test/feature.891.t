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
use Time::Local;
use Test::More tests => 34;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
}

# Feature 891: UUID filter should be uuid.endswith by default
# Create some example data directly.  This is so that we have complete control
# over the UUID.
if (open my $fh, '>', 'pending.data')
{
  my $timeA = timegm (00,00,12,22,11,2008);
  my $timeB = timegm (00,00,12,17,03,2009);
  print $fh <<EOF;
[description:"one" entry:"$timeA" status:"pending" uuid:"a7097693-91c2-4cbe-ba89-ddcc87e5582c"]
[description:"two" entry:"$timeB" status:"pending" uuid:"e8f72d91-964c-424b-8fd5-556434648b6b"]
EOF

  close $fh;
}

my $output = qx{../src/task rc:$rc 1 info 2>&1};
my ($uuid) = $output =~ /UUID\s+(\S{36})/ms;

$output = qx{../src/task rc:$rc $uuid list 2>&1};
like ($output, qr/one/, "Found with $uuid");

my ($short) = $uuid =~ /^(.{35})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{34})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{33})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{32})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{31})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{30})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{29})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{28})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{27})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{26})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{25})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{24})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{23})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{22})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{21})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{20})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{19})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{18})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{17})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{16})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{15})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{14})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{13})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{12})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{11})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{10})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{9})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{8})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
like ($output, qr/one/, "Found with $short");

($short) = $uuid =~ /^(.{7})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{6})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{5})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{4})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
unlike ($output, qr/one/, "Not found with $short");

($short) = $uuid =~ /^(.{3})/;
$output = qx{../src/task rc:$rc $short list 2>&1};
unlike ($output, qr/one/, "Not found with $short");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

