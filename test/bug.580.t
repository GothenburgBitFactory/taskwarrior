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
use Test::More tests => 11;
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
unlink qw(local/pending.data local/completed.data local/undo.data local/undo.save local/backlog.data local/synch.key local.rc);
ok (! -r 'local/pending.data'   &&
    ! -r 'local/completed.data' &&
    ! -r 'local/undo.data'      &&
    ! -r 'local/undo.save'      &&
    ! -r 'local/backlog.data'   &&
    ! -r 'local/synch.key'      &&
    ! -r 'local.rc', 'Cleanup');

unlink qw(remote/pending.data remote/completed.data remote/undo.data remote/backlog.data remote/synch.key remote.rc);
ok (! -r 'remote/pending.data'   &&
    ! -r 'remote/completed.data' &&
    ! -r 'remote/undo.data'      &&
    ! -r 'remote/backlog.data'   &&
    ! -r 'remote/synch.key'      &&
    ! -r 'remote.rc', 'Cleanup');

rmdir("remote/extensions");
ok (!-e "remote/extensions", "Removed dir remote/extensions");

rmdir("remote");
ok (!-e "remote", "Removed dir remote");

rmdir("local/extensions");
ok (!-e "local/extensions", "Removed dir local/extensions");

rmdir("local");
ok (!-e "local", "Removed dir local");

exit 0;
