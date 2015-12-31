#! /usr/bin/perl
################################################################################
##
## Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
use Getopt::Long;

# Give a nice error if the (non-standard) JSON module is not installed.
eval "use JSON";
if ($@)
{
  print "Error: You need to install the JSON Perl module.\n";
  exit 1;
}

eval "use LWP::Simple";
if ($@)
{
  print "Error: You need to install the LWP::Simple Perl module.\n";
  exit 1;
}

eval "use Encode";
if ($@)
{
  print "Error: You need to install the Encode Perl module.\n";
  exit 1;
}

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

Typical usage is to simply specify a locale, like this:

  ./update-holidays.pl --locale en-US --file holidays.en-US.rc

This will give you all holidays for the locale.  If you want data for a locale
that has regional data, you may want to specify regions, to make sure you get
the data local to your region.  For example, the following command will download
Australian holiday data that is either national, or specific to the New South
Wales territory.

  update-holidays.pl --locale en-AU \
                     --region NSW \
                     --file holidays.en-AU.rc

Multiple regions may be specified, such as the two Swiss cantons of Zürich and
Schwyz:

  update-holidays.pl --locale de-CH \
                     --region Zürich \
                     --region Schwyz \
                     --file holidays.de-CH.rc

See http://holidata.net for details of supported locales and regions.

It is recommended that you regularly update your holiday files.  Not only does
this keep your holiday data current, but allows for corrected data to be used.

EOF

  exit 1;
}

# File name is required.
die "You must specify a holiday file to update.\n"  if ! $file;

# Convert @regions to %region_hash to simplify lookup.
my %region_hash = map {$_ => undef} @regions;

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

# Fetch data for the current year.
my $data_current = get ($url_current);
print "\n",
      "Data for ${locale}, for ${current} could not be downloaded.  This could\n",
      "mean that you do not have an internet connection, or that\n",
      "Holidata.net does not support this locale and/or year.\n",
      "\n"
  unless defined $data_current;

# Fetch data for the next year.
my $data_next = get ($url_next);
print "\n",
      "Data for ${locale}, for ${next} could not be downloaded.  This could\n",
      "mean that you do not have an internet connection, or that\n",
      "Holidata.net does not support this locale and/or year.\n",
      "\n"
  unless defined $data_next;

# Without data, cannot proceed.
my $data;
$data .= decode ('utf-8', $data_current) if defined $data_current;
$data .= decode ('utf-8', $data_next)    if defined $data_next;
exit (1) if !defined $data || $data eq '';

# Filter the holidays according to @regions.
my $id = 1;
my $content;
my %seen;
for my $holiday (split /\n/ms, $data)
{
  my $parsed = from_json ($holiday);

  # Change date format from YYYY-MM-DD to YYYYMMDD.
  $parsed->{'date'} =~ s/-//g;

  if (@regions == 0 ||
      (@regions > 0 && ($parsed->{'region'} eq '' ||
                        exists $region_hash{$parsed->{'region'}})))
  {
    next if $seen{ $parsed->{'description'} . ':' . $parsed->{'date'} }++;
    $content .= "holiday.${locale}${id}.name=" . $parsed->{'description'} .  "\n" .
                "holiday.${locale}${id}.date=" . $parsed->{'date'} .         "\n";
  }

  ++$id;
}

# Overwrite the file.
if (open my $fh, '>:utf8', $file)
{
  print $fh
        "###############################################################################\n",
        "# International Holiday Data provided by Holidata.net\n",
        "# ${url_current}\n",
        "# ${url_next}\n",
        "#\n",
        "# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.\n",
        "#\n",
        "# Permission is hereby granted, free of charge, to any person obtaining a copy\n",
        "# of this software and associated documentation files (the \"Software\"), to deal\n",
        "# in the Software without restriction, including without limitation the rights\n",
        "# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n",
        "# copies of the Software, and to permit persons to whom the Software is\n",
        "# furnished to do so, subject to the following conditions:\n",
        "#\n",
        "# The above copyright notice and this permission notice shall be included\n",
        "# in all copies or substantial portions of the Software.\n",
        "#\n",
        "# THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n",
        "# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n",
        "# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL\n",
        "# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n",
        "# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n",
        "# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n",
        "# SOFTWARE.\n",
        "#\n",
        "# http://www.opensource.org/licenses/mit-license.php\n",
        "#\n",
        "###############################################################################\n",
        "\n",
        $content;

  close $fh;
}

exit 0;

################################################################################

