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
use Test::More tests => 12;

# Create the rc file.
if (open my $fh, '>', 'pri.rc')
{
  print $fh "data.location=.\n",
            "report.foo.description=DESC\n",
            "report.foo.columns=id,priority.long\n",
            "report.foo.labels=ID,P\n",
            "report.foo.sort=id+\n";
  close $fh;
  ok (-r 'pri.rc', 'Created pri.rc');
}

# Generate the usage screen, and locate the custom report on it.
qx{../src/task rc:pri.rc add one   pri:H};
qx{../src/task rc:pri.rc add two   pri:M};
qx{../src/task rc:pri.rc add three pri:L};
qx{../src/task rc:pri.rc add four  pri:};

my $output = qx{../src/task rc:pri.rc foo 2>&1};
like ($output,   qr/ID.+Pri/,    'priority.long indicator heading');
like ($output,   qr/1\s+High/,   'priority.long High');
like ($output,   qr/2\s+Medium/, 'priority.long Medium');
like ($output,   qr/3\s+Low/,    'priority.long Low');
like ($output,   qr/4\s*\n/,     'priority.long None');

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

unlink 'pri.rc';
ok (!-r 'pri.rc', 'Removed pri.rc');

exit 0;

