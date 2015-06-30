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
use Test::More tests => 21;

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
            "confirmation=on\n";
  close $fh;
}

# Create the response file.
if (open my $fh, '>', 'response.txt')
{
  print $fh "\n\nn\n";
  close $fh;
}

qx{../src/task rc:$rc add foo 2>&1} for 1..10;

# Test the various forms of "Yes".
my $output = qx{echo "Yes" | ../src/task rc:$rc 1 del 2>&1};
like ($output, qr/Delete task 1 'foo'\? \(yes\/no\)/, 'confirmation - Yes works');
unlike ($output, qr/Task not deleted\./, 'confirmation - Yes works');

$output = qx{echo "ye" | ../src/task rc:$rc 2 del 2>&1};
like ($output, qr/Delete task 2 'foo'\? \(yes\/no\)/, 'confirmation - ye works');
unlike ($output, qr/Task not deleted\./, 'confirmation - ye works');

$output = qx{echo "y" | ../src/task rc:$rc 3 del 2>&1};
like ($output, qr/Delete task 3 'foo'\? \(yes\/no\)/, 'confirmation - y works');
unlike ($output, qr/Task not deleted\./, 'confirmation - y works');

$output = qx{echo "YES" | ../src/task rc:$rc 4 del 2>&1};
like ($output, qr/Delete task 4 'foo'\? \(yes\/no\)/, 'confirmation - YES works');
unlike ($output, qr/Task not deleted\./, 'confirmation - YES works'); # 10

$output = qx{echo "YE" | ../src/task rc:$rc 5 del 2>&1};
like ($output, qr/Delete task 5 'foo'\? \(yes\/no\)/, 'confirmation - YE works');
unlike ($output, qr/Task not deleted\./, 'confirmation - YE works');

$output = qx{echo "Y" | ../src/task rc:$rc 6 del 2>&1};
like ($output, qr/Delete task 6 'foo'\? \(yes\/no\)/, 'confirmation - Y works');
unlike ($output, qr/Task not deleted\./, 'confirmation - Y works');

# Test the various forms of "no".
$output = qx{echo "no" | ../src/task rc:$rc 7 del 2>&1};
like ($output, qr/Delete task 7 'foo'\? \(yes\/no\)/, 'confirmation - no works');
like ($output, qr/Task not deleted\./, 'confirmation - no works');

$output = qx{echo "n" | ../src/task rc:$rc 7 del 2>&1};
like ($output, qr/Delete task 7 'foo'\? \(yes\/no\)/, 'confirmation - n works');
like ($output, qr/Task not deleted\./, 'confirmation - n works');

$output = qx{echo "NO" | ../src/task rc:$rc 7 del 2>&1};
like ($output, qr/Delete task 7 'foo'\? \(yes\/no\)/, 'confirmation - NO works');
like ($output, qr/Task not deleted\./, 'confirmation - NO works'); # 20

$output = qx{echo "N" | ../src/task rc:$rc 7 del 2>&1};
like ($output, qr/Delete task 7 'foo'\? \(yes\/no\)/, 'confirmation - N works');
like ($output, qr/Task not deleted\./, 'confirmation - N works');

# Test newlines.
$output = qx{cat response.txt | ../src/task rc:$rc 7 del 2>&1};
like ($output, qr/(Delete task 7 'foo'\? \(yes\/no\)) \1 \1/, 'confirmation - \n re-prompt works'); # 43

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data response.txt), $rc;
exit 0;

