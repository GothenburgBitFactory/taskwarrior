#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 26;

# Create the rc file.
if (open my $fh, '>', 'abbrev.rc')
{
  print $fh "data.location=.\n",
            "abbreviation.minimum=1\n";
  close $fh;
  ok (-r 'abbrev.rc', 'Created abbrev.rc');
}

# Test the priority attribute abbrevations.
qx{../src/task rc:abbrev.rc add priority:H with};
qx{../src/task rc:abbrev.rc add without};

my $output = qx{../src/task rc:abbrev.rc list priority:H};
like   ($output, qr/\bwith\b/,    'priority:H with');
unlike ($output, qr/\bwithout\b/, 'priority:H without');

$output = qx{../src/task rc:abbrev.rc list priorit:H};
like   ($output, qr/\bwith\b/,    'priorit:H with');
unlike ($output, qr/\bwithout\b/, 'priorit:H without');

$output = qx{../src/task rc:abbrev.rc list priori:H};
like   ($output, qr/\bwith\b/,    'priori:H with');
unlike ($output, qr/\bwithout\b/, 'priori:H without');

$output = qx{../src/task rc:abbrev.rc list prior:H};
like   ($output, qr/\bwith\b/,    'prior:H with');
unlike ($output, qr/\bwithout\b/, 'prior:H without');

$output = qx{../src/task rc:abbrev.rc list prio:H};
like   ($output, qr/\bwith\b/,    'prio:H with');
unlike ($output, qr/\bwithout\b/, 'prio:H without');

$output = qx{../src/task rc:abbrev.rc list pri:H};
like   ($output, qr/\bwith\b/,    'pri:H with');
unlike ($output, qr/\bwithout\b/, 'pri:H without');

# Test the version command abbreviations.
$output = qx{../src/task rc:abbrev.rc version};
like ($output, qr/MIT\s+license/, 'version');

$output = qx{../src/task rc:abbrev.rc versio};
like ($output, qr/MIT\s+license/, 'version');

$output = qx{../src/task rc:abbrev.rc versi};
like ($output, qr/MIT\s+license/, 'version');

$output = qx{../src/task rc:abbrev.rc vers};
like ($output, qr/MIT\s+license/, 'version');

$output = qx{../src/task rc:abbrev.rc ver};
like ($output, qr/MIT\s+license/, 'version');

$output = qx{../src/task rc:abbrev.rc ve};
like ($output, qr/MIT\s+license/, 'version');

$output = qx{../src/task rc:abbrev.rc v};
like ($output, qr/MIT\s+license/, 'version');

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

unlink 'abbrev.rc';
ok (!-r 'abbrev.rc', 'Removed abbrev.rc');

exit 0;

