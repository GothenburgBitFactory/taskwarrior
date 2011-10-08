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
use Test::More tests => 15;

# Create the rc file.
if (open my $fh, '>', 'font.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'font.rc', 'Created font.rc');
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

qx{../src/task rc:font.rc add foo};
my $output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:off rc.fontunderline:off};
like   ($output, qr/--------/, '0,0,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:off rc.fontunderline:on};
like   ($output, qr/--------/, '0,0,1 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:on rc.fontunderline:off};
like   ($output, qr/--------/, '0,1,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:off rc._forcecolor:on rc.fontunderline:on};
unlike ($output, qr/--------/, '0,1,1 -> underline');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:off rc.fontunderline:off};
like   ($output, qr/--------/, '1,0,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:off rc.fontunderline:on};
like   ($output, qr/--------/, '1,0,1 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:on rc.fontunderline:off};
like   ($output, qr/--------/, '1,1,0 -> dashes');
$output = qx{../src/task 1 info rc:font.rc rc.color:on rc._forcecolor:on rc.fontunderline:on};
unlike ($output, qr/--------/, '1,1,1 -> underline');

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

unlink 'font.rc';
ok (!-r 'font.rc', 'Removed font.rc');

exit 0;

