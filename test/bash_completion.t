#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  print $fh "alias.samplealias=long\n";

  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Copy task.sh and make substitutions & additions needed for testing.
if (open my $target, '>', 'task.sh')
{
  if (open my $source, '<', '../scripts/bash/task.sh')
  {
    while (<$source>)
    {
      my $temp=$_;
      chomp($_);
      if ($_ eq qw{taskcommand='task'})
      {
        print $target "taskcommand='../src/task rc:bug.rc'";
      }
      else
      {
        print $target $temp;
      }
    }
    close ($source);
    print $target 'COMP_WORDS=("$@")',
                  "\n",
                  'COMP_CWORD=$(($#-1))',
                  "\n",
                  '_task',
                  "\n",
                  'for reply_iter in "${COMPREPLY[@]}"; do',
                  "\n",
                  '  echo $reply_iter',
                  "\n",
                  'done';
    close $target;
    ok (-r 'task.sh', 'Created task.sh');
  }
}

# aliases should be expanded
my $output = qx{bash ./task.sh task sampleali};
ok ($? == 0, 'Exit status check');
like ($output, qr/samplealias/, 'Aliases are expanded');

$output = qx{bash ./task.sh task m};
ok ($? == 0, 'Exit status check');
like ($output, qr/modify/, 'expansion of \'m\' includes \'modify\'');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc task.sh);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc'         &&
    ! -r 'task.sh', 'Cleanup');

exit 0;
