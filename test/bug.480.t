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
use Test::More tests => 37;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "defaultwidth=0\n";

  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug #480 - putting a '@' character in tags breaks filters.
qx{../src/task rc:bug.rc add one +ordinary};
qx{../src/task rc:bug.rc add two +\@strange};

my $output = qx{../src/task rc:bug.rc long +ordinary};
like ($output,   qr/one/, '+ordinary explicitly included'); # 2
unlike ($output, qr/two/, '@strange implicitly excluded');

$output = qx{../src/task rc:bug.rc long -ordinary};
unlike ($output, qr/one/, '-ordinary explicitly excluded');
like ($output,   qr/two/, '@strange implicitly included'); # 5

$output = qx{../src/task rc:bug.rc long +\@strange};
unlike ($output, qr/one/, '-ordinary implicitly excluded');
like ($output,   qr/two/, '@strange explicitly included'); # 7

$output = qx{../src/task rc:bug.rc long -\@strange};
like ($output,   qr/one/, '+ordinary implicitly included'); # 8
unlike ($output, qr/two/, '@strange explicitly excluded');

# Bug #XXX - '-t1 -t2' doesn't seem to work, when @ characters are involved.
unlink 'pending.data';
qx{../src/task rc:bug.rc add one   +t1};
qx{../src/task rc:bug.rc add two   +t2};
qx{../src/task rc:bug.rc add three +t3};

$output = qx{../src/task rc:bug.rc list -t1};
unlike ($output, qr/one/,   'Single: no t1');
like   ($output, qr/two/,   'Single: yes t2');
like   ($output, qr/three/, 'Single: yes t3');

$output = qx{../src/task rc:bug.rc list -t1 -t2};
unlike ($output, qr/one/,   'Double: no t1');
unlike ($output, qr/two/,   'Double: no t2');
like   ($output, qr/three/, 'Double: yes t3');

$output = qx{../src/task rc:bug.rc list -t1 -t2 -t3};
unlike ($output, qr/one/,   'Triple: no t1');
unlike ($output, qr/two/,   'Triple: no t2');
unlike ($output, qr/three/, 'Triple: no t3');

# Once again, with @ characters.
qx{../src/task rc:bug.rc 1 modify +\@1};
qx{../src/task rc:bug.rc 2 modify +\@2};
qx{../src/task rc:bug.rc 3 modify +\@3};

$output = qx{../src/task rc:bug.rc list -\@1};
unlike ($output, qr/one/,   'Single: no @1');    # 19
like   ($output, qr/two/,   'Single: yes @2');
like   ($output, qr/three/, 'Single: yes @3');

$output = qx{../src/task rc:bug.rc list -\@1 -\@2};
unlike ($output, qr/one/,   'Double: no @1');
unlike ($output, qr/two/,   'Double: no @2');
like   ($output, qr/three/, 'Double: yes @3');

$output = qx{../src/task rc:bug.rc list -\@1 -\@2 -\@3};
unlike ($output, qr/one/,   'Triple: no @1');
unlike ($output, qr/two/,   'Triple: no @2');
unlike ($output, qr/three/, 'Triple: no @3');

# Once again, with @ characters and punctuation.
qx{../src/task rc:bug.rc 1 modify +\@foo.1};
qx{../src/task rc:bug.rc 2 modify +\@foo.2};
qx{../src/task rc:bug.rc 3 modify +\@foo.3};

$output = qx{../src/task rc:bug.rc list -\@foo.1};
unlike ($output, qr/one/,   'Single: no @foo.1');
like   ($output, qr/two/,   'Single: yes @foo.2');
like   ($output, qr/three/, 'Single: yes @foo.3');

$output = qx{../src/task rc:bug.rc list -\@foo.1 -\@foo.2};
unlike ($output, qr/one/,   'Double: no @foo.1');
unlike ($output, qr/two/,   'Double: no @foo.2');
like   ($output, qr/three/, 'Double: yes @foo.3');

$output = qx{../src/task rc:bug.rc list -\@foo.1 -\@foo.2 -\@foo.3};
unlike ($output, qr/one/,   'Triple: no @foo.1');
unlike ($output, qr/two/,   'Triple: no @foo.2');
unlike ($output, qr/three/, 'Triple: no @foo.3');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;
