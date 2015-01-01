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
use Test::More tests => 7;

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
            "confirmation=off\n",
            "color=off\n",
            "verbose=nothing\n";
  close $fh;
}

# Bug 1056: Project indentation in CmdSummary.
qx{../src/task rc:$rc add testing project:existingParent 2>&1 >/dev/null};
qx{../src/task rc:$rc add testing project:existingParent.child 2>&1 >/dev/null};
qx{../src/task rc:$rc add testing project:abstractParent.kid 2>&1 >/dev/null};
qx{../src/task rc:$rc add testing project:.myProject 2>&1 >/dev/null};
qx{../src/task rc:$rc add testing project:myProject. 2>&1 >/dev/null};
qx{../src/task rc:$rc add testing project:.myProject. 2>&1 >/dev/null};

my $output = qx{../src/task rc:$rc summary 2>&1};
my @lines = split ('\n',$output);

like ($lines[0], qr/^\.myProject\s/,       "$ut: '.myProject'        not indented");
like ($lines[1], qr/^\.myProject\.\s/,     "$ut: '.myProject.'       not indented");
like ($lines[2], qr/^abstractParent\s*$/,  "$ut: 'abstractParent'    not indented, no data");
like ($lines[3], qr/^\s\skid\s+\d/,        "$ut: '  kid'             indented, without parent, with data");
like ($lines[4], qr/^existingParent\s+\d/, "$ut: 'existingParent'    not indented, with data");
like ($lines[5], qr/^\s\schild\s+\d/,      "$ut: '  child'           indented, without parent, with data");
like ($lines[6], qr/^myProject\.\s+\d/,    "$ut: 'myProject.'        not indented, with data");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

