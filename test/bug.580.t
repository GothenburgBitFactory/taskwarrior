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
qx{../src/task rc:remote.rc add remote task 2>&1};

# add a local task
qx{../src/task rc:local.rc add local task 2>&1};

# merge and autopush
qx{../src/task rc:local.rc merge 2>&1};

my $output = qx{../src/task rc:remote.rc ls 2>&1};
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
