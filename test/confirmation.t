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
use Test::More tests => 50;

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
  print $fh "-- \n\nn\n";
  close $fh;
  ok (-r 'response.txt', 'Created response.txt');
}

qx{../src/task rc:confirm.rc add foo} for 1..10;

# Test the various forms of "Yes".
my $output = qx{echo "-- Yes" | ../src/task rc:confirm.rc 1 del};
like ($output, qr/Permanently delete task 1 'foo'\? \(y\/n\)/, 'confirmation - Yes works');
unlike ($output, qr/Task not deleted\./, 'confirmation - Yes works');

$output = qx{echo "-- ye" | ../src/task rc:confirm.rc 2 del};
like ($output, qr/Permanently delete task 2 'foo'\? \(y\/n\)/, 'confirmation - ye works');
unlike ($output, qr/Task not deleted\./, 'confirmation - ye works');

$output = qx{echo "-- y" | ../src/task rc:confirm.rc 3 del};
like ($output, qr/Permanently delete task 3 'foo'\? \(y\/n\)/, 'confirmation - y works');
unlike ($output, qr/Task not deleted\./, 'confirmation - y works');

$output = qx{echo "-- YES" | ../src/task rc:confirm.rc 4 del};
like ($output, qr/Permanently delete task 4 'foo'\? \(y\/n\)/, 'confirmation - YES works');
unlike ($output, qr/Task not deleted\./, 'confirmation - YES works');

$output = qx{echo "-- YE" | ../src/task rc:confirm.rc 5 del};
like ($output, qr/Permanently delete task 5 'foo'\? \(y\/n\)/, 'confirmation - YE works');
unlike ($output, qr/Task not deleted\./, 'confirmation - YE works');

$output = qx{echo "-- Y" | ../src/task rc:confirm.rc 6 del};
like ($output, qr/Permanently delete task 6 'foo'\? \(y\/n\)/, 'confirmation - Y works');
unlike ($output, qr/Task not deleted\./, 'confirmation - Y works');

# Test the various forms of "no".
$output = qx{echo "-- no" | ../src/task rc:confirm.rc 7 del};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - no works');
like ($output, qr/Task not deleted\./, 'confirmation - no works');

$output = qx{echo "-- n" | ../src/task rc:confirm.rc 7 del};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - n works');
like ($output, qr/Task not deleted\./, 'confirmation - n works');

$output = qx{echo "-- NO" | ../src/task rc:confirm.rc 7 del};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - NO works');
like ($output, qr/Task not deleted\./, 'confirmation - NO works');

$output = qx{echo "-- N" | ../src/task rc:confirm.rc 7 del};
like ($output, qr/Permanently delete task 7 'foo'\? \(y\/n\)/, 'confirmation - N works');
like ($output, qr/Task not deleted\./, 'confirmation - N works');

# Test Yes for multiple changes
$output = qx{echo -e "-- y\nY\nY\nY\nY" | ../src/task rc:confirm.rc 7-10 modify +bar};
like ($output, qr/Proceed with change\? \(yes\/no\/all\/quit\)/, 'multiple confirmations - Y works');
like ($output, qr/Task 7 "foo"/,     'multiple confirmations - Y works');
like ($output, qr/Task 8 "foo"/,     'multiple confirmations - Y works');
like ($output, qr/Task 9 "foo"/,     'multiple confirmations - Y works');
like ($output, qr/Task 10 "foo"/,    'multiple confirmations - Y works');
like ($output, qr/Modified 4 tasks/, 'multiple confirmations - Y works');

# Test no for multiple changes
$output = qx{echo -e "-- N\nn\nn\nn\nn" | ../src/task rc:confirm.rc 7-10 modify -bar};
like ($output, qr/Proceed with change\? \(yes\/no\/all\/quit\)/, 'multiple confirmations - n works');
like ($output, qr/Task 7 "foo"/,     'multiple confirmations - n works');
like ($output, qr/Task 8 "foo"/,     'multiple confirmations - n works');
like ($output, qr/Task 9 "foo"/,     'multiple confirmations - n works');
like ($output, qr/Task 10 "foo"/,    'multiple confirmations - n works');
like ($output, qr/Modified 0 tasks/, 'multiple confirmations - n works');

# Test All for multiple changes
$output = qx{echo -e "-- a\nA" | ../src/task rc:confirm.rc 7-10 modify -bar};
like ($output, qr/Proceed with change\? \(yes\/no\/all\/quit\)/, 'multiple confirmations - A works');
like ($output,   qr/Task 7 "foo"/,     'multiple confirmations - A works');
unlike ($output, qr/Task 8 "foo"/,     'multiple confirmations - A works');
like ($output,   qr/Modified 4 tasks/, 'multiple confirmations - A works');

# Test quit for multiple changes
$output = qx{echo "-- q" | ../src/task rc:confirm.rc 7-10 modify +bar};
like ($output, qr/Proceed with change\? \(yes\/no\/all\/quit\)/, 'multiple confirmations - q works');
like ($output,   qr/Task 7 "foo"/,     'multiple confirmations - q works');
unlike ($output, qr/Task 8 "foo"/,     'multiple confirmations - q works');
like ($output,   qr/Modified 0 tasks/, 'multiple confirmations - q works');

# Test newlines.
$output = qx{cat response.txt | ../src/task rc:confirm.rc 7 del};
like ($output, qr/(Permanently delete task 7 'foo'\? \(y\/n\)) \1 \1/, 'confirmation - \n re-prompt works');

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

unlink 'response.txt';
ok (!-r 'response.txt', 'Removed response.txt');

unlink 'confirm.rc';
ok (!-r 'confirm.rc', 'Removed confirm.rc');

exit 0;

