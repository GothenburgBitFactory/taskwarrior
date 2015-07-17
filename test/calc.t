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
use Test::More tests => 23;

# '15min' is seen as '15', 'min', not '15min' duration.
my $output = qx{../src/calc --debug '12 * 3600 + 34 * 60 + 56'};
like ($output, qr/Eval literal number ↑'12'/,   'Number 12');
like ($output, qr/Eval literal number ↑'3600'/, 'Number 3600');
like ($output, qr/Eval literal number ↑'60'/,   'Number 60');
like ($output, qr/Eval literal number ↑'60'/,   'Number 60');
like ($output, qr/Eval literal number ↑'56'/,   'Number 56');
like ($output, qr/^45296$/ms,                   'Result 45296');
unlike ($output, qr/Error/,                     'No errors');

$output = qx{../src/calc --debug --postfix '12 3600 * 34 60 * 56 + +'};
like ($output, qr/Eval literal number ↑'12'/,   'Number 12');
like ($output, qr/Eval literal number ↑'3600'/, 'Number 3600');
like ($output, qr/Eval literal number ↑'60'/,   'Number 60');
like ($output, qr/Eval literal number ↑'60'/,   'Number 60');
like ($output, qr/Eval literal number ↑'56'/,   'Number 56');
like ($output, qr/^45296$/ms,                   'Result 45296');
unlike ($output, qr/Error/,                     'No errors');

$output = qx{../src/calc --debug '2- -3'};
like ($output, qr/Eval literal number ↑'2'/ms,  'Number 2');
like ($output, qr/Eval _neg_ ↓'3' → ↑'-3'/ms,   'Operator -');
like ($output, qr/Eval literal number ↑'2'/ms,  'Number 3');
like ($output, qr/^5$/ms,                       'Result 5');
unlike ($output, qr/Error/,                     'No errors');

$output = qx{../src/calc --help};
like ($output, qr/Usage:/,                      '--help shows usage');
like ($output, qr/Options:/,                    '--help shows options');

$output = qx{../src/calc --version};
like ($output, qr/calc \d\.\d+\.\d+/,           '--version shows version');
like ($output, qr/Copyright/,                   '--version shows Copyright');

exit 0;

