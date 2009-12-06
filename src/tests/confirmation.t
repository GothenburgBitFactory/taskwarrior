#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2009, Paul Beckingham.
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
use Test::More tests => 27;

# Create the rc file.
if (open my $fh, '>', 'confirm.rc')
{
  print $fh "data.location=.\n",
            "confirmation=yes\n";
  close $fh;
  ok (-r 'confirm.rc', 'Created confirm.rc');
}

# Create the response file.
if (open my $fh, '>', 'response.txt')
{
  print $fh "\n\nn\n";
  close $fh;
  ok (-r 'response.txt', 'Created response.txt');
}

qx{../task rc:confirm.rc add foo} for 1 .. 10;

# Test the various forms of "Yes".
my $output = qx{echo "Yes" | ../task rc:confirm.rc del 1};
like ($output, qr/Permanently delete task 1 'foo'\? \(y\/n\)/, 'confirmation - Yes works');
unlike ($output, qr/Task not deleted\./, 'confirmation - Yes works');

$output = qx{echo "ye" | ../task rc:confirm.rc del 2};
like ($output, qr/Permanently delete task 2 'foo'\? \(y\/n\)/, 'confirmation - ye works');
unlike ($output, qr/Task not deleted\./, 'confirmation - ye works');

$output = qx{echo "y" | ../task rc:confirm.rc del 3};
like ($output, qr/Permanently delete task 3 'foo'\? \(y\/n\)/, 'confirmation - y works');
unlike ($output, qr/Task not deleted\./, 'confirmation - y works');

$output = qx{echo "YES" | ../task rc:confirm.rc del 4};
like ($output, qr/Permanently delete task 4 'foo'\? \(y\/n\)/, 'confirmation - YES works');
unlike ($output, qr/Task not deleted\./, 'confirmation - YES works');

$output = qx{echo "YE" | ../task rc:confirm.rc del 5};
like ($output, qr/Permanently delete task 5 'foo'\? \(y\/n\)/, 'confirmation - YE works');
unlike ($output, qr/Task not deleted\./, 'confirmation - YE works');

$output = qx{echo "Y" | ../task rc:confirm.rc del 6};
like ($output, qr/Permanently delete task 6 'foo'\? \(y\/n\)/, 'confirmation - Y works');
unlike ($output, qr/Task not deleted\./, 'confirmation - Y works');

# Test the various forms of "no".
$output = qx{echo "no" | ../task rc:confirm.rc del 7};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - no works');
like ($output, qr/Task not deleted\./, 'confirmation - no works');

$output = qx{echo "n" | ../task rc:confirm.rc del 7};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - n works');
like ($output, qr/Task not deleted\./, 'confirmation - n works');

$output = qx{echo "NO" | ../task rc:confirm.rc del 7};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - NO works');
like ($output, qr/Task not deleted\./, 'confirmation - NO works');

$output = qx{echo "N" | ../task rc:confirm.rc del 7};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - N works');
like ($output, qr/Task not deleted\./, 'confirmation - N works');

# Test newlines.
$output = qx{cat response.txt | ../task rc:confirm.rc del 7};
like ($output, qr/(Permanently delete task 7 'foo'\? \(y\/n\)) \1 \1/, 'confirmation - \n re-prompt works');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'response.txt';
ok (!-r 'response.txt', 'Removed response.txt');

unlink 'confirm.rc';
ok (!-r 'confirm.rc', 'Removed confirm.rc');

exit 0;

