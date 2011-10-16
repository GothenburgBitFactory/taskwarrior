#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 21;

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
unlink qw(pending.data completed.data undo.data backlog.data synch.key abbrev.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'abbrev.rc', 'Cleanup');

exit 0;

