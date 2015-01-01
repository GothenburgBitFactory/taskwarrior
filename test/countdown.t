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
use Test::More tests => 80;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'countdown.rc')
{
  print $fh "data.location=.\n";
  print $fh "report.up.description=countdown+ report\n";
  print $fh "report.up.columns=id,due.countdown,description\n";
  print $fh "report.up.labels=ID,Countdown,Description\n";
  print $fh "report.up.filter=status:pending\n";
  print $fh "report.up.sort=due+\n";

  print $fh "report.down.description=countdown- report\n";
  print $fh "report.down.columns=id,due.countdown,description\n";
  print $fh "report.down.labels=ID,Countdown,Description\n";
  print $fh "report.down.filter=status:pending\n";
  print $fh "report.down.sort=due-\n";

  print $fh "report.upc.description=countdown_compact+ report\n";
  print $fh "report.upc.columns=id,due.countdown,description\n";
  print $fh "report.upc.labels=ID,Countdown,Description\n";
  print $fh "report.upc.filter=status:pending\n";
  print $fh "report.upc.sort=due+\n";

  print $fh "report.downc.description=countdown_compact- report\n";
  print $fh "report.downc.columns=id,due.countdown,description\n";
  print $fh "report.downc.labels=ID,Countdown,Description\n";
  print $fh "report.downc.filter=status:pending\n";
  print $fh "report.downc.sort=due-\n";

  close $fh;
}

# Create a variety of pending tasks with increasingly higher due dates
# to ensure proper sort order.
qx{../src/task rc:countdown.rc add one       due:-1.2years 2>&1};
qx{../src/task rc:countdown.rc add two       due:-9months 2>&1};
qx{../src/task rc:countdown.rc add three     due:-2months 2>&1};
qx{../src/task rc:countdown.rc add four      due:-3weeks 2>&1};
qx{../src/task rc:countdown.rc add five      due:-7days 2>&1};
qx{../src/task rc:countdown.rc add six       due:-2days 2>&1};
qx{../src/task rc:countdown.rc add seven     due:-1day 2>&1};
qx{../src/task rc:countdown.rc add eight     due:-12hours 2>&1};
qx{../src/task rc:countdown.rc add nine      due:-6hours 2>&1};
qx{../src/task rc:countdown.rc add ten       due:-1hour 2>&1};
qx{../src/task rc:countdown.rc add eleven    due:-30seconds 2>&1};
qx{../src/task rc:countdown.rc add twelve    due:1hour 2>&1};
qx{../src/task rc:countdown.rc add thirteen  due:6hours 2>&1};
qx{../src/task rc:countdown.rc add fourteen  due:12hours 2>&1};
qx{../src/task rc:countdown.rc add fifteen   due:1days 2>&1};
qx{../src/task rc:countdown.rc add sixteen   due:2days 2>&1};
qx{../src/task rc:countdown.rc add seventeen due:7days 2>&1};
qx{../src/task rc:countdown.rc add eighteen  due:3weeks 2>&1};
qx{../src/task rc:countdown.rc add nineteen  due:2months 2>&1};
qx{../src/task rc:countdown.rc add twenty    due:9months 2>&1};
qx{../src/task rc:countdown.rc add twentyone due:1.2years 2>&1};

my $output = qx{../src/task rc:countdown.rc up 2>&1};
like ($output, qr/\bone\b.+\btwo\b/ms,          'up: one < two');
like ($output, qr/\btwo\b.+\bthree/ms,          'up: two < three');
like ($output, qr/\bthree\b.+\bfour/ms,         'up: three < four');
like ($output, qr/\bfour\b.+\bfive/ms,          'up: four < five');
like ($output, qr/\bfive\b.+\bsix/ms,           'up: five < six');
like ($output, qr/\bsix\b.+\bseven/ms,          'up: six < seven');
like ($output, qr/\bseven\b.+\beight/ms,        'up: seven < eight');
like ($output, qr/\beight\b.+\bnine/ms,         'up: eight < nine');
like ($output, qr/\bnine\b.+\bten/ms,           'up: nine < ten');
like ($output, qr/\bten\b.+\beleven/ms,         'up: ten < eleven');
like ($output, qr/\beleven\b.+\btwelve/ms,      'up: eleven < twelve');
like ($output, qr/\btwelve\b.+\bthirteen/ms,    'up: twelve < thirteen');
like ($output, qr/\bthirteen\b.+\bfourteen/ms,  'up: thirteen < fourteen');
like ($output, qr/\bfourteen\b.+\bfifteen/ms,   'up: fourteen < fifteen');
like ($output, qr/\bfifteen\b.+\bsixteen/ms,    'up: fifteen < sixteen');
like ($output, qr/\bsixteen\b.+\bseventeen/ms,  'up: sixteen < seventeen');
like ($output, qr/\bseventeen\b.+\beighteen/ms, 'up: seventeen < eighteen');
like ($output, qr/\beighteen\b.+\bnineteen/ms,  'up: eighteen < nineteen');
like ($output, qr/\bnineteen\b.+\btwenty/ms,    'up: nineteen < twenty');
like ($output, qr/\btwenty\b.+\btwentyone/ms,   'up: twenty < twentyone');

$output = qx{../src/task rc:countdown.rc down 2>&1};
like ($output, qr/\btwentyone\b.+\btwenty/ms,   'down: twentyone < twenty');
like ($output, qr/\btwenty\b.+\bnineteen/ms,    'down: twenty < nineteen');
like ($output, qr/\bnineteen\b.+\beighteen/ms,  'down: nineteen < eighteen');
like ($output, qr/\beighteen\b.+\bseventeen/ms, 'down: eighteen < seventeen');
like ($output, qr/\bseventeen\b.+\bsixteen/ms,  'down: seventeen < sixteen');
like ($output, qr/\bsixteen\b.+\bfifteen/ms,    'down: sixteen < fifteen');
like ($output, qr/\bfifteen\b.+\bfourteen/ms,   'down: fifteen < fourteen');
like ($output, qr/\bfourteen\b.+\bthirteen/ms,  'down: fourteen < thirteen');
like ($output, qr/\bthirteen\b.+\btwelve/ms,    'down: thirteen < twelve');
like ($output, qr/\btwelve\b.+\beleven/ms,      'down: twelve < eleven');
like ($output, qr/\beleven\b.+\bten/ms,         'down: eleven < ten');
like ($output, qr/\bten\b.+\bnine/ms,           'down: ten < nine');
like ($output, qr/\bnine\b.+\beight/ms,         'down: nine < eight');
like ($output, qr/\beight\b.+\bseven/ms,        'down: eight < seven');
like ($output, qr/\bseven\b.+\bsix/ms,          'down: seven < six');
like ($output, qr/\bsix\b.+\bfive/ms,           'down: six < five');
like ($output, qr/\bfive\b.+\bfour/ms,          'down: five < four');
like ($output, qr/\bfour\b.+\bthree/ms,         'down: four < three');
like ($output, qr/\bthree\b.+\btwo/ms,          'down: three < two');
like ($output, qr/\btwo\b.+\bone\b/ms,          'down: two < one');

$output = qx{../src/task rc:countdown.rc upc 2>&1};
like ($output, qr/\bone\b.+\btwo\b/ms,          'upc: one < two');
like ($output, qr/\btwo\b.+\bthree/ms,          'upc: two < three');
like ($output, qr/\bthree\b.+\bfour/ms,         'upc: three < four');
like ($output, qr/\bfour\b.+\bfive/ms,          'upc: four < five');
like ($output, qr/\bfive\b.+\bsix/ms,           'upc: five < six');
like ($output, qr/\bsix\b.+\bseven/ms,          'upc: six < seven');
like ($output, qr/\bseven\b.+\beight/ms,        'upc: seven < eight');
like ($output, qr/\beight\b.+\bnine/ms,         'upc: eight < nine');
like ($output, qr/\bnine\b.+\bten/ms,           'upc: nine < ten');
like ($output, qr/\bten\b.+\beleven/ms,         'upc: ten < eleven');
like ($output, qr/\beleven\b.+\btwelve/ms,      'upc: eleven < twelve');
like ($output, qr/\btwelve\b.+\bthirteen/ms,    'upc: twelve < thirteen');
like ($output, qr/\bthirteen\b.+\bfourteen/ms,  'upc: thirteen < fourteen');
like ($output, qr/\bfourteen\b.+\bfifteen/ms,   'upc: fourteen < fifteen');
like ($output, qr/\bfifteen\b.+\bsixteen/ms,    'upc: fifteen < sixteen');
like ($output, qr/\bsixteen\b.+\bseventeen/ms,  'upc: sixteen < seventeen');
like ($output, qr/\bseventeen\b.+\beighteen/ms, 'upc: seventeen < eighteen');
like ($output, qr/\beighteen\b.+\bnineteen/ms,  'upc: eighteen < nineteen');
like ($output, qr/\bnineteen\b.+\btwenty/ms,    'upc: nineteen < twenty');
like ($output, qr/\btwenty\b.+\btwentyone/ms,   'upc: twenty < twentyone');

$output = qx{../src/task rc:countdown.rc downc 2>&1};
like ($output, qr/\btwentyone\b.+\btwenty/ms,   'downc: twentyone < twenty');
like ($output, qr/\btwenty\b.+\bnineteen/ms,    'downc: twenty < nineteen');
like ($output, qr/\bnineteen\b.+\beighteen/ms,  'downc: nineteen < eighteen');
like ($output, qr/\beighteen\b.+\bseventeen/ms, 'downc: eighteen < seventeen');
like ($output, qr/\bseventeen\b.+\bsixteen/ms,  'downc: seventeen < sixteen');
like ($output, qr/\bsixteen\b.+\bfifteen/ms,    'downc: sixteen < fifteen');
like ($output, qr/\bfifteen\b.+\bfourteen/ms,   'downc: fifteen < fourteen');
like ($output, qr/\bfourteen\b.+\bthirteen/ms,  'downc: fourteen < thirteen');
like ($output, qr/\bthirteen\b.+\btwelve/ms,    'downc: thirteen < twelve');
like ($output, qr/\btwelve\b.+\beleven/ms,      'downc: twelve < eleven');
like ($output, qr/\beleven\b.+\bten/ms,         'downc: eleven < ten');
like ($output, qr/\bten\b.+\bnine/ms,           'downc: ten < nine');
like ($output, qr/\bnine\b.+\beight/ms,         'downc: nine < eight');
like ($output, qr/\beight\b.+\bseven/ms,        'downc: eight < seven');
like ($output, qr/\bseven\b.+\bsix/ms,          'downc: seven < six');
like ($output, qr/\bsix\b.+\bfive/ms,           'downc: six < five');
like ($output, qr/\bfive\b.+\bfour/ms,          'downc: five < four');
like ($output, qr/\bfour\b.+\bthree/ms,         'downc: four < three');
like ($output, qr/\bthree\b.+\btwo/ms,          'downc: three < two');
like ($output, qr/\btwo\b.+\bone\b/ms,          'downc: two < one');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data countdown.rc);
exit 0;
