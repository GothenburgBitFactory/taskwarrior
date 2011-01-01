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
use Test::More tests => 16;
use File::Copy;

use constant false => 0;
use constant true => 1;

# Create data locations
mkdir("local", 0755);
ok(-e 'local', "Created directory local");
mkdir("remote", 0755);
ok(-e 'remote', "Created directory remote");

# Create the rc files.
if (open my $fh, '>', 'local.rc')
{
  print $fh "data.location=./local\n",
            "confirmation=no\n",
				"merge.default.uri=remote/\n",
				"merge.autopush=yes\n";
  close $fh;
  ok (-r 'local.rc', 'Created local.rc');
}

if (open my $fh, '>', 'remote.rc')
{
  print $fh "data.location=./remote\n",
            "confirmation=no\n",
				"merge.autopush=no\n";
  close $fh;
  ok (-r 'remote.rc', 'Created remote.rc');
}

# add a remote task
qx{../src/task rc:remote.rc add remote task};

# add a local task
qx(../src/task rc:local.rc add local task);

# merge and autopush
qx{../src/task rc:local.rc merge};

my $output = qx{../src/task rc:remote.rc ls};
like ($output, qr/local task/,    "autopush failed");

# Cleanup.
unlink 'local/pending.data';
ok (!-r 'local/pending.data', 'Removed local/pending.data');

unlink 'local/completed.data';
ok (!-r 'local/completed.data', 'Removed local/completed.data');

unlink 'local/undo.data';
ok (!-r 'local/undo.data', 'Removed local/undo.data');

unlink 'local/undo.save';
ok (!-r 'local/undo.save', 'Removed local/undo.save');

unlink 'local.rc';
ok (!-r 'local.rc', 'Removed local.rc');

unlink 'remote/pending.data';
ok (!-r 'remote/pending.data', 'Removed remote/pending.data');

unlink 'remote/completed.data';
ok (!-r 'remote/completed.data', 'Removed remote/completed.data');

unlink 'remote/undo.data';
ok (!-r 'remote/undo.data', 'Removed remote/undo.data');

unlink 'remote.rc';
ok (!-r 'remote.rc', 'Removed remote.rc');

rmdir("remote");
ok (!-e "remote", "Removed dir remote");
rmdir("local");
ok (!-e "local", "Removed dir local");

exit 0;
