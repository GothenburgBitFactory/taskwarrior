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

