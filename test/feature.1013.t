#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 10;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'outerr.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'outerr.rc', 'Created outerr.rc');
}

# Feature 1013: output error, header, footnote and debug messages on standard
# error

# Check that errors are sent to standard error
my $stdout = qx{../src/task rc:outerr.rc add due:__ foo 2> /dev/null};
unlike ($stdout, qr/^The duration '__' was not recognized as valid, with correct units like '3days'.$/ms, 'Errors are not sent to stdout');
my $stderr = qx{../src/task rc:outerr.rc add due:__ bar 2>&1 >/dev/null};
like ($stderr, qr/^The duration '__' was not recognized as valid, with correct units like '3days'.$/ms, 'Errors are sent to stderr');

# Check that headers are sent to standard error
$stdout = qx{../src/task rc:outerr.rc list 2> /dev/null};
unlike ($stdout, qr/^Using alternate .taskrc file .+outerr.rc$/ms, 'Headers are not sent to stdout');
$stderr = qx{../src/task rc:outerr.rc list 2>&1 >/dev/null};
like ($stderr, qr/^Using alternate .taskrc file .+outerr.rc$/ms, 'Headers are sent to stderr');

# Check that footnotes are sent to standard error
$stdout = qx{../src/task rc:outerr.rc rc.debug:on list 2> /dev/null};
unlike ($stdout, qr/^Configuration override rc.debug:on$/ms, 'Footnotes are not sent to stdout');
$stderr = qx{../src/task rc:outerr.rc rc.debug:on list 2>&1 >/dev/null};
like ($stderr, qr/^Configuration override rc.debug:on$/ms, 'Footnotes are sent to stderr');

# Check that debugs are sent to standard error
$stdout = qx{../src/task rc:outerr.rc rc.debug:on list 2> /dev/null};
unlike ($stdout, qr/^Timer Config::load \(.+outerr.rc\) /ms, 'Debugs are not sent to stdout');
$stderr = qx{../src/task rc:outerr.rc rc.debug:on list 2>&1 >/dev/null};
like ($stderr, qr/^Timer Config::load \(.+outerr.rc\) /ms, 'Debugs are sent to stderr');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data outerr.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'outerr.rc', 'Cleanup');

exit 0;

