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
use Test::More tests => 5;

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
            "default.command=\n";
  close $fh;
}

# Get the version number from configure.ac
my $version = slurp ('../CMakeLists.txt');

# Test the usage command.
my $output = qx{../src/task rc:$rc 2>&1 >/dev/null};
like ($output, qr/You must specify a command or a task to modify./m, "$ut: missing command and ID");

# Test the version command.
$output = qx{../src/task rc:$rc version 2>&1};
like ($output, qr/task $version/, "$ut: version - task version number");
like ($output, qr/MIT\slicense/, "$ut: version - license");
like ($output, qr/http:\/\/taskwarrior\.org/, "$ut: version - url");

# Test the _version command.
$output = qx{../src/task rc:$rc _version 2>&1};
like ($output, qr/[a-f0-9]{7}/, "$ut: _version - task version number");

# Cleanup.
unlink $rc;
exit 0;

################################################################################
sub slurp
{
  my ($file) = @_;
  if (open my $fh, '<', $file)
  {
    while (<$fh>) {
      if (/PROJECT_VERSION/) {
        chomp;
        s/^set \(PROJECT_VERSION "//;
        s/"\).*$//;
        close  $fh;
        return $_;
      }
    }
  }
  '';
}
