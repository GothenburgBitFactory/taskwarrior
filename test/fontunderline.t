#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 8;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'font.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

# Test the fontunderline config variable.  The following truth table defines
# the different results which are to be confirmed.
#
#   color  _forcecolor  fontunderline  result
#   -----  -----------  -------------  ---------
#       0            0              0  dashes
#       0            0              1  dashes
#       0            1              0  dashes
#       0            1              1  underline
#       1*           0              0  dashes
#       1*           0              1  dashes
#       1*           1              0  dashes
#       1*           1              1  underline
#
# * When isatty (fileno (stdout)) is false, color is automatically disabled.

qx{../src/task rc:font.rc add foo 2>&1};
my $output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:off rc.fontunderline:off 2>&1};
like   ($output, qr/--------/, '0,0,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:off rc.fontunderline:on 2>&1};
like   ($output, qr/--------/, '0,0,1 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:on rc.fontunderline:off 2>&1};
like   ($output, qr/--------/, '0,1,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:on rc.fontunderline:on 2>&1};
unlike ($output, qr/--------/, '0,1,1 -> underline');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:off rc.fontunderline:off 2>&1};
like   ($output, qr/--------/, '1,0,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:off rc.fontunderline:on 2>&1};
like   ($output, qr/--------/, '1,0,1 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:on rc.fontunderline:off 2>&1};
like   ($output, qr/--------/, '1,1,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:on rc.fontunderline:on 2>&1};
unlike ($output, qr/--------/, '1,1,1 -> underline');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data font.rc);
exit 0;

