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
use Test::More tests => 24;

# Create the rc file.
if (open my $fh, '>', 'shadow.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "shadow.file=./shadow.txt\n",
            "shadow.command=rc:shadow.rc stats\n",
            "shadow.notify=on\n";
  close $fh;
  ok (-r 'shadow.rc', 'Created shadow.rc');
}

my $output = qx{../src/task rc:shadow.rc add one};
like ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file updated on add');

$output = qx{../src/task rc:shadow.rc list};
unlike ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file not updated on list');

$output = qx{../src/task rc:shadow.rc 1 delete};
like ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file updated on delete');

$output = qx{../src/task rc:shadow.rc list};
unlike ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file not updated on list');

# Inspect the shadow file.
my $file = slurp ('./shadow.txt');
like ($file, qr/Pending\s+0\n/,                        'Pending 0');
like ($file, qr/Recurring\s+0\n/,                      'Recurring 0');
like ($file, qr/Completed\s+0\n/,                      'Completed 0');
like ($file, qr/Deleted\s+1\n/,                        'Deleted 1');
like ($file, qr/Total\s+1\n/,                          'Total 1');
like ($file, qr/Task used for\s+-\n/,                  'Task used for -');
like ($file, qr/Task added every\s+-\n/,               'Task added every -');
like ($file, qr/Task deleted every\s+-\n/,             'Task deleted every -');
like ($file, qr/Average desc length\s+3 characters\n/, 'Average desc length 3 characters');
like ($file, qr/Tasks tagged\s+0%\n/,                  'Tasks tagged 0%');
like ($file, qr/Unique tags\s+0\n/,                    'Unique tags 0');
like ($file, qr/Projects\s+0\n/,                       'Projects 0');

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

unlink 'shadow.rc';
ok (!-r 'shadow.rc', 'Removed shadow.rc');

unlink 'shadow.txt';
ok (!-r 'shadow.txt', 'Removed shadow.txt');

exit 0;

################################################################################
sub slurp
{
  my ($file) = @_;
  local $/;

  if (open my $fh, '<', $file)
  {
    my $contents = <$fh>;
    close $fh;
    return $contents;
  }

  '';
}

