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
use Test::More tests => 37;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "defaultwidth=0\n",
            "confirmation=off\n";

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
