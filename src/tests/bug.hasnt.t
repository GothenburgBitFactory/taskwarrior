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
use Test::More tests => 19;

# Create the rc file.
if (open my $fh, '>', 'hasnt.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'hasnt.rc', 'Created hasnt.rc');
}

# 1
qx{../task rc:hasnt.rc add foo};

# 2
qx{../task rc:hasnt.rc add foo};
qx{../task rc:hasnt.rc 2 annotate bar};

# 3
qx{../task rc:hasnt.rc add foo};
qx{../task rc:hasnt.rc 3 annotate bar};
sleep 2;
qx{../task rc:hasnt.rc 3 annotate baz};

# 4
qx{../task rc:hasnt.rc add bar};

# 5
qx{../task rc:hasnt.rc add bar};
qx{../task rc:hasnt.rc 5 annotate foo};

# 6
qx{../task rc:hasnt.rc add bar};
qx{../task rc:hasnt.rc 6 annotate foo};
sleep 2;
qx{../task rc:hasnt.rc 6 annotate baz};

#7
qx{../task rc:hasnt.rc add one};
qx{../task rc:hasnt.rc 7 annotate two};
sleep 2;
qx{../task rc:hasnt.rc 7 annotate three};

my $output = qx{../task rc:hasnt.rc ls description.has:foo};
like   ($output, qr/\n 1/, '1 has foo -> yes');
like   ($output, qr/\n 2/, '2 has foo -> yes');
like   ($output, qr/\n 3/, '3 has foo -> yes');
unlike ($output, qr/\n 4/, '4 has foo -> no');
like   ($output, qr/\n 5/, '5 has foo -> yes');
like   ($output, qr/\n 6/, '6 has foo -> yes');
unlike ($output, qr/\n 7/, '7 has foo -> no');

$output = qx{../task rc:hasnt.rc ls description.hasnt:foo};
unlike ($output, qr/\n 1/, '1 hasnt foo -> no');
unlike ($output, qr/\n 2/, '2 hasnt foo -> no');
unlike ($output, qr/\n 3/, '3 hasnt foo -> no');
like   ($output, qr/\n 4/, '4 hasnt foo -> yes');
unlike ($output, qr/\n 5/, '5 hasnt foo -> no');
unlike ($output, qr/\n 6/, '6 hasnt foo -> no');
like   ($output, qr/\n 7/, '7 hasnt foo -> yes');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'hasnt.rc';
ok (!-r 'hasnt.rc', 'Removed hasnt.rc');

exit 0;

