#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 38;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "defaultwidth=0\n",
            "confirmation=off\n";

  close $fh;
}

# Bug #480 - putting a '@' character in tags breaks filters.
qx{../src/task rc:$rc add one +ordinary 2>&1};
qx{../src/task rc:$rc add two +\@strange 2>&1};

my $output = qx{../src/task rc:$rc long +ordinary 2>&1};
like ($output,   qr/one/, "$ut: +ordinary explicitly included");
unlike ($output, qr/two/, "$ut: \@strange implicitly excluded");

$output = qx{../src/task rc:$rc long -ordinary 2>&1};
unlike ($output, qr/one/, "$ut: -ordinary explicitly excluded");
like ($output,   qr/two/, "$ut: \@strange implicitly included");

$output = qx{../src/task rc:$rc long +\@strange 2>&1};
unlike ($output, qr/one/, "$ut: -ordinary implicitly excluded");
like ($output,   qr/two/, "$ut: \@strange explicitly included");

$output = qx{../src/task rc:$rc long -\@strange 2>&1};
like ($output,   qr/one/, "$ut: +ordinary implicitly included");
unlike ($output, qr/two/, "$ut: \@strange explicitly excluded");

# Bug #XXX - '-t1 -t2' doesn't seem to work, when @ characters are involved.
unlink 'pending.data';
qx{../src/task rc:$rc add one   +t1 2>&1};
qx{../src/task rc:$rc add two   +t2 2>&1};
qx{../src/task rc:$rc add three +t3 2>&1};

$output = qx{../src/task rc:$rc list -t1 2>&1};
unlike ($output, qr/one/,   "$ut: Single: no t1");
like   ($output, qr/two/,   "$ut: Single: yes t2");
like   ($output, qr/three/, "$ut: Single: yes t3");

$output = qx{../src/task rc:$rc list -t1 -t2 2>&1};
unlike ($output, qr/one/,   "$ut: Double: no t1");
unlike ($output, qr/two/,   "$ut: Double: no t2");
like   ($output, qr/three/, "$ut: Double: yes t3");

$output = qx{../src/task rc:$rc list -t1 -t2 -t3 2>&1};
like ($output, qr/^No matches.$/m, "$ut: No task listed");
unlike ($output, qr/one/,   "$ut: Triple: no t1");
unlike ($output, qr/two/,   "$ut: Triple: no t2");
unlike ($output, qr/three/, "$ut: Triple: no t3");

# Once again, with @ characters.
qx{../src/task rc:$rc 1 modify +\@1 2>&1};
qx{../src/task rc:$rc 2 modify +\@2 2>&1};
qx{../src/task rc:$rc 3 modify +\@3 2>&1};

$output = qx{../src/task rc:$rc list -\@1 2>&1};
unlike ($output, qr/one/,   "$ut: Single: no \@1");
like   ($output, qr/two/,   "$ut: Single: yes \@2");
like   ($output, qr/three/, "$ut: Single: yes \@3");

$output = qx{../src/task rc:$rc list -\@1 -\@2 2>&1};
unlike ($output, qr/one/,   "$ut: Double: no \@1");
unlike ($output, qr/two/,   "$ut: Double: no \@2");
like   ($output, qr/three/, "$ut: Double: yes \@3");

$output = qx{../src/task rc:$rc list -\@1 -\@2 -\@3 2>&1};
like ($output, qr/^No matches.$/m, "$ut: No task listed");
unlike ($output, qr/one/,   "$ut: Triple: no \@1");
unlike ($output, qr/two/,   "$ut: Triple: no \@2");
unlike ($output, qr/three/, "$ut: Triple: no \@3");

# Once again, with @ characters and punctuation.
qx{../src/task rc:$rc 1 modify +\@foo.1 2>&1};
qx{../src/task rc:$rc 2 modify +\@foo.2 2>&1};
qx{../src/task rc:$rc 3 modify +\@foo.3 2>&1};

$output = qx{../src/task rc:$rc list -\@foo.1 2>&1};
unlike ($output, qr/one/,   "$ut: Single: no \@foo.1");
like   ($output, qr/two/,   "$ut: Single: yes \@foo.2");
like   ($output, qr/three/, "$ut: Single: yes \@foo.3");

$output = qx{../src/task rc:$rc list -\@foo.1 -\@foo.2 2>&1};
unlike ($output, qr/one/,   "$ut: Double: no \@foo.1");
unlike ($output, qr/two/,   "$ut: Double: no \@foo.2");
like   ($output, qr/three/, "$ut: Double: yes \@foo.3");

$output = qx{../src/task rc:$rc list -\@foo.1 -\@foo.2 -\@foo.3 2>&1};
like ($output, qr/^No matches.$/m, "$ut: No task listed");
unlike ($output, qr/one/,   "$ut: Triple: no \@foo.1");
unlike ($output, qr/two/,   "$ut: Triple: no \@foo.2");
unlike ($output, qr/three/, "$ut: Triple: no \@foo.3");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
