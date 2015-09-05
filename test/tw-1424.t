#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# http://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import datetime
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class Test1424(TestCase):
    def setUp(self):
        self.t = Task()

    def test_1824_days(self):
        """Check that due:1824d works"""
        self.t('add foo due:1824d')
        code, out, err = self.t('_get 1.due.year')
        # NOTE This test has a possible race condition when run "during" EOY.
        # If Taskwarrior is executed at 23:59:59 on new year's eve and the
        # python code below runs at 00:00:00 on new year's day, the two will
        # disagree on the proper year. Using libfaketime with a frozen time
        # or the date set to $year-01-01 might be a good idea here.
        plus_1824d = datetime.datetime.today() + datetime.timedelta(days=1824)
        self.assertEqual(out, "%d\n" % (plus_1824d.year))

    def test_3648_days(self):
        """Check that due:3648d works"""
        self.t('add foo due:3648d')
        code, out, err = self.t('_get 1.due.year')
        # NOTE This test has a possible race condition when run "during" EOY.
        # If Taskwarrior is executed at 23:59:59 on new year's eve and the
        # python code below runs at 00:00:00 on new year's day, the two will
        # disagree on the proper year. Using libfaketime with a frozen time
        # or the date set to $year-01-01 might be a good idea here.
        plus_3648d = datetime.datetime.today() + datetime.timedelta(days=3648)
        self.assertEqual(out, "%d\n" % (plus_3648d.year))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
