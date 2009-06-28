#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2009, Paul Beckingham.
## All rights reserved.
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the
##
##     Free Software Foundation, Inc.,
##     51 Franklin Street, Fifth Floor,
##     Boston, MA
##     02110-1301
##     USA
##
################################################################################

use strict;
use warnings;
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'export.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'export.rc', 'Created export.rc');
}

# Add two tasks, export, examine result.
qx{../task rc:export.rc add priority:H project:A one};
qx{../task rc:export.rc add +tag1 +tag2 two};
qx{../task rc:export.rc export > ./export.txt};

my @lines;
if (open my $fh, '<', './export.txt')
{
  while (my $line = <$fh>)
  {
    next unless $line =~ /^['0-9]/;
    push @lines, $line;
  }

  close $fh;
}

my $line1 = qr/'id','uuid','status','tags','entry','start','due','recur','end','project','priority','fg','bg','description'\n/;
my $line2 = qr/1,'.{8}-.{4}-.{4}-.{4}-.{12}','pending','',\d+,,,,,'A','H',,,'one'\n/;
my $line3 = qr/2,'.{8}-.{4}-.{4}-.{4}-.{12}','pending','tag1 tag2',\d+,,,,,,,,,'two'\n/;

like ($lines[0], $line1, "export line one");
like ($lines[1], $line2, "export line two");
like ($lines[2], $line3, "export line three");

# Cleanup.
unlink 'export.txt';
ok (!-r 'export.txt', 'Removed export.txt');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'export.rc';
ok (!-r 'export.rc', 'Removed export.rc');

exit 0;

