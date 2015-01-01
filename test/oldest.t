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
use Test::More tests => 48;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n";
  close $fh;
}

# Create 11 tasks, progressively 1 second newer.
if (open my $fh, '>', 'pending.data')
{
  my $entry = time () - 3600;  # An hour ago.

  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000001" status:"pending" description:"one" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000002" status:"pending" description:"two" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000003" status:"pending" description:"three" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000004" status:"pending" description:"four" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000005" status:"pending" description:"five" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000006" status:"pending" description:"six" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000007" status:"pending" description:"seven" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000008" status:"pending" description:"eight" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000009" status:"pending" description:"nine" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000010" status:"pending" description:"ten" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000011" status:"pending" description:"eleven" entry:"$entry"]\n};
  $entry++;

  close $fh;
}

my $output = qx{../src/task rc:$rc oldest limit:10 2>&1};
ok ($? == 0,                  "$ut: oldest limit:10");
like   ($output, qr/ one/,    "$ut: oldest limit:10: one");
like   ($output, qr/ two/,    "$ut: oldest limit:10: two");
like   ($output, qr/ three/,  "$ut: oldest limit:10: three");
like   ($output, qr/ four/,   "$ut: oldest limit:10: four");
like   ($output, qr/ five/,   "$ut: oldest limit:10: five");
like   ($output, qr/ six/,    "$ut: oldest limit:10: six");
like   ($output, qr/ seven/,  "$ut: oldest limit:10: seven");
like   ($output, qr/ eight/,  "$ut: oldest limit:10: eight");
like   ($output, qr/ nine/,   "$ut: oldest limit:10: nine");
like   ($output, qr/ ten/,    "$ut: oldest limit:10: ten");
unlike ($output, qr/ eleven/, "$ut: oldest limit:10: ! eleven");

$output = qx{../src/task rc:$rc oldest limit:3 2>&1};
ok ($? == 0,                  "$ut: oldest limit:3");
like   ($output, qr/ one/,    "$ut: oldest limit:3: one");
like   ($output, qr/ two/,    "$ut: oldest limit:3: two");
like   ($output, qr/ three/,  "$ut: oldest limit:3: three");
unlike ($output, qr/ four/,   "$ut: oldest limit:3: ! four");
unlike ($output, qr/ five/,   "$ut: oldest limit:3: ! five");
unlike ($output, qr/ six/,    "$ut: oldest limit:3: ! six");
unlike ($output, qr/ seven/,  "$ut: oldest limit:3: ! seven");
unlike ($output, qr/ eight/,  "$ut: oldest limit:3: ! eight");
unlike ($output, qr/ nine/,   "$ut: oldest limit:3: ! nine");
unlike ($output, qr/ ten/,    "$ut: oldest limit:3: ! ten");
unlike ($output, qr/ eleven/, "$ut: oldest limit:3: ! eleven");

$output = qx{../src/task rc:$rc newest limit:10 2>&1};
ok ($? == 0,                  "$ut: newest limit:10");
unlike ($output, qr/ one/,    "$ut: newest limit:10: ! one");
like   ($output, qr/ two/,    "$ut: newest limit:10: two");
like   ($output, qr/ three/,  "$ut: newest limit:10: three");
like   ($output, qr/ four/,   "$ut: newest limit:10: four");
like   ($output, qr/ five/,   "$ut: newest limit:10: five");
like   ($output, qr/ six/,    "$ut: newest limit:10: six");
like   ($output, qr/ seven/,  "$ut: newest limit:10: seven");
like   ($output, qr/ eight/,  "$ut: newest limit:10: eight");
like   ($output, qr/ nine/,   "$ut: newest limit:10: nine");
like   ($output, qr/ ten/,    "$ut: newest limit:10: ten");
like   ($output, qr/ eleven/, "$ut: newest limit:10: eleven");

$output = qx{../src/task rc:$rc newest limit:3 2>&1};
ok ($? == 0,                  "$ut: newest limit:3");
unlike ($output, qr/ one/,    "$ut: newest limit:3: ! one");
unlike ($output, qr/ two/,    "$ut: newest limit:3: ! two");
unlike ($output, qr/ three/,  "$ut: newest limit:3: ! three");
unlike ($output, qr/ four/,   "$ut: newest limit:3: ! four");
unlike ($output, qr/ five/,   "$ut: newest limit:3: ! five");
unlike ($output, qr/ six/,    "$ut: newest limit:3: ! six");
unlike ($output, qr/ seven/,  "$ut: newest limit:3: ! seven");
unlike ($output, qr/ eight/,  "$ut: newest limit:3: ! eight");
like   ($output, qr/ nine/,   "$ut: newest limit:3: nine");
like   ($output, qr/ ten/,    "$ut: newest limit:3: ten");
like   ($output, qr/ eleven/, "$ut: newest limit:3: eleven");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

