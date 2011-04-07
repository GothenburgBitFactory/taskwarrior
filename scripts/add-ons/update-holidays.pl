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
use Getopt::Long;
use JSON;
use LWP::Simple;

# Command line options, argument validation.
my $help;
my $locale;
my @regions;
my $file;
Getopt::Long::GetOptions (
  'help'     => \$help,
  'locale=s' => \$locale,
  'region=s' => \@regions,
  'file=s'   => \$file);

if ($help || !$file)
{
  print <<EOF;

usage: update-holidays.pl [--help]
                          [--locale <locale>]
                          [--region <region>]
                          --file <holiday-file>

  --help         Displays this message.
  --locale       Locale to be used, if it cannot be derived from the file name.
  --region       Regional data.  May be specified more than once.
  --file         Location of the holiday file to update.
                 Note: this file *will* be overwritten.

EOF

  exit 1;
}

# File name is required.
die "You must specify a holiday file to update.\n"  if ! $file;

# Perhaps the locale can be found in the file name, if not already specified?
if (!$locale &&
    $file =~ /([a-z]{2}-[A-Z]{2})/)
{
  $locale = $1;
}

die "The locale could not be determined from the file name.  " .
    "Please secify a locale.\n"  if !$locale;

# Determine the current year.
my $current = (localtime (time))[5] + 1900;
my $next    = $current + 1;

# Construct the holidata.net URL.
my $url_current = "http://holidata.net/${locale}/${current}.json";
my $url_next    = "http://holidata.net/${locale}/${next}.json";

# Fetch the data.
my $data;
eval {$data = get ($url_current);};
eval {$data .= get ($url_next);};
die "Could not query data for ${locale}, for ${next}.\n" unless defined $data;

# Filter the holidays according to @regions.
my $id = 1;
my $content;
for my $holiday (split /\n/ms, $data)
{
  my $parsed = from_json ($holiday);

  $content .= "holiday.${locale}${id}.name=" . $parsed->{'description'} .  "\n" .
              "holiday.${locale}${id}.date=" . $parsed->{'date'} .         "\n";

  ++$id;
}

# Overwrite the file.
if (open my $fh, '>:encoding(UTF-8)', $file)
{
  print $fh
        "# International Holiday Data provided by Holidata.net\n",
        "#\n",
        "# ${url_current}\n",
        "#\n",
        $content;

  close $fh;
}

exit 0;

################################################################################

