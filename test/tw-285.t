#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
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

import sys
import os
import signal
from glob import glob
# Ensure python finds the local simpletap and basetest modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import BaseTestCase


class BaseTest285(BaseTestCase):
    @classmethod
    def prepare(cls):
        with open("bug.rc", 'w') as fh:
            fh.write("data.location=.\n"
                     "verbose=nothing\n"
                     "confirmation=no\n")

    def setUp(self):
        """Executed before each test in the class"""

        #              OVERDUE YESTERDAY DUE TODAY TOMORROW WEEK MONTH YEAR
        # due:-1week      Y       -       -    -       -      ?    ?     ?
        # due:-1day       Y       Y       -    -       -      ?    ?     ?
        # due:today       Y       -       Y    Y       -      ?    ?     ?
        # due:tomorrow    -       -       Y    -       Y      ?    ?     ?
        # due:3days       -       -       Y    -       -      ?    ?     ?
        # due:1month      -       -       -    -       -      -    -     ?
        # due:1year       -       -       -    -       -      -    -     -

        self.callTaskSuccess(['rc:bug.rc', 'add', 'due_last_week',     'due:-1week'])
        self.callTaskSuccess(['rc:bug.rc', 'add', 'due_yesterday',     'due:-1day'])
        self.callTaskSuccess(['rc:bug.rc', 'add', 'due_earlier_today', 'due:today'])
        self.callTaskSuccess(['rc:bug.rc', 'add', 'due_later_today',   'due:tomorrow'])
        self.callTaskSuccess(['rc:bug.rc', 'add', 'due_three_days',    'due:3days'])
        self.callTaskSuccess(['rc:bug.rc', 'add', 'due_next_month',    'due:1month'])
        self.callTaskSuccess(['rc:bug.rc', 'add', 'due_next_year',     'due:1year'])

    def tearDown(self):
        """Needed after each test or setUp will cause duplicated data at start
        of the next test.
        """
        for file in glob("*.data"):
            os.remove(file)

    @classmethod
    def finish(cls):
        os.remove("bug.rc")
        for file in glob("*.data"):
            os.remove(file)


class Test285(BaseTest285):
    def test_overdue(self):
        """+OVERDUE"""
        code, out, err = self.callTaskSuccess(["rc:bug.rc", "+OVERDUE", "count"])
        self.assertEqual(out, "3\n", "+OVERDUE == 3 tasks")

    def test_yesterday(self):
        """+YESTERDAY"""
        code, out, err = self.callTaskSuccess(["rc:bug.rc", "+YESTERDAY", "count"])
        self.assertEqual(out, "1\n", "+YESTERDAY == 1 task")

    def test_due(self):
        """+DUE"""
        code, out, err = self.callTaskSuccess(["rc:bug.rc", "+DUE", "count"])
        self.assertEqual(out, "3\n", "+DUE == 3 task")

    def test_today(self):
        """+TODAY"""
        code, out, err = self.callTaskSuccess(["rc:bug.rc", "+TODAY", "count"])
        self.assertEqual(out, "1\n", "+TODAY == 1 task")

    def test_duetoday(self):
        """+DUETODAY"""
        code, out, err = self.callTaskSuccess(["rc:bug.rc", "+DUETODAY", "count"])
        self.assertEqual(out, "1\n", "+DUETODAY == 1 task")

    def test_tomorrow(self):
        """+TOMORROW"""
        code, out, err = self.callTaskSuccess(["rc:bug.rc", "+TOMORROW", "count"])
        self.assertEqual(out, "1\n", "+TOMORROW == 1 task")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    import unittest
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
