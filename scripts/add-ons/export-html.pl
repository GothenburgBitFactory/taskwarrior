#! /usr/bin/perl
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

# Give a nice error if the (non-standard) JSON module is not installed.
eval "use JSON";
if ($@)
{
  print "Error: You need to install the JSON Perl module.\n";
  exit 1;
}

# Use the taskwarrior 2.0+ export command to filter and return JSON
my $command = join (' ', ("env PATH='$ENV{PATH}' task export", @ARGV));
if ($command =~ /No matches/)
{
  printf STDERR $command;
  exit 1;
}

# Generate output.
print "<html>\n",
      "  <body>\n",
      "    <table>\n",
      "      <thead>\n",
      "        <tr>\n",
      "          <td>ID</td>\n",
      "          <td>Pri</td>\n",
      "          <td>Description</td>\n",
      "          <td>Project</td>\n",
      "          <td>Due</td>\n",
      "        </tr>\n",
      "      </thead>\n",
      "      <tbody>\n";

my $count = 0;
for my $task (split /,$/ms, qx{$command})
{
  ++$count;
  my $data = from_json ($task);

  print "        <tr>\n",
        "          <td>", ($data->{'id'}          || ''), "</td>\n",
        "          <td>", ($data->{'priority'}    || ''), "</td>\n",
        "          <td>", ($data->{'description'} || ''), "</td>\n",
        "          <td>", ($data->{'project'}     || ''), "</td>\n",
        "          <td>", ($data->{'due'}         || ''), "</td>\n",
        "        </tr>\n";
}

print "      </tbody>\n",
      "      <tfooter>\n",
      "        <tr>\n",
      "          <td>", $count, " matching tasks</td>\n",
      "        </tr>\n",
      "      </tfooter>\n",
      "    </table>\n",
      "  </body>\n",
      "</html>\n";

exit 0;

################################################################################

