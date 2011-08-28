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

# Give a nice error if the (non-standard) JSON module is not installed.
eval "use JSON";
if ($@)
{
  print "Error: You need to install the JSON Perl module.\n";
  exit 1;
}

# Use the taskwarrior 2.0+ export command to filter and return JSON
my $command = join (' ', ("env PATH=$ENV{PATH} task export", @ARGV));
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

