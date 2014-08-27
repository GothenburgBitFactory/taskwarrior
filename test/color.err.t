#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 4;

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "color.header=blue\n",
            "color.footnote=red\n",
            "color.error=yellow\n",
            "color.debug=green\n",
            "_forcecolor=1\n";
  close $fh;
}

# Test the errors colors
my $output = qx{../src/task rc:$rc rc.debug:on add foo priority:X 2>&1 >/dev/null};
like ($output, qr/^\033\[33mThe 'priority' attribute does not allow a value of 'X'\./ms, "$ut: color.error");
like ($output, qr/^\033\[32mTimer Config::load \(.*?$rc\)/ms,                            "$ut: color.debug");
like ($output, qr/^\033\[34mUsing alternate \.taskrc file/ms,                            "$ut: color.header");
like ($output, qr/^\033\[31mConfiguration override rc\.debug:on/ms,                      "$ut: color.footnote");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

