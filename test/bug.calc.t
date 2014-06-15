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

# '15min' is seen as '15', 'min', not '15min' duration.
my $output = qx{../src/calc --debug 15min};
unlike ($output, qr/token infix '15' Date/,           'Misinterpretation: 15min -> 15');
unlike ($output, qr/token infix 'min' Identifier/,    'Misinterpretation: 15min -> m');
unlike ($output, qr/Error: Unexpected stack size: 2/, 'Unexpected stack size');
like ($output, qr/\[1\] eval push 'PT15M' Duration/,  '15min -> PT15M Duration');
like ($output, qr/PT15M/,                             '15min -> PT15m Duration');

exit 0;

