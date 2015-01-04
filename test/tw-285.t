#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
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

import sys
import os
import unittest
# Ensure python finds the local simpletap and basetest modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class Test285(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()
        cls.t.config("verbose", "nothing")

        #              OVERDUE YESTERDAY DUE TODAY TOMORROW WEEK MONTH YEAR
        # due:-1week      Y       -       -    -       -      ?    ?     ?
        # due:-1day       Y       Y       -    -       -      ?    ?     ?
        # due:today       Y       -       Y    Y       -      ?    ?     ?
        # due:tomorrow    -       -       Y    -       Y      ?    ?     ?
        # due:3days       -       -       Y    -       -      ?    ?     ?
        # due:1month      -       -       -    -       -      -    -     ?
        # due:1year       -       -       -    -       -      -    -     -

        cls.t(('add', 'due_last_week',     'due:-1week'))
        cls.t(('add', 'due_yesterday',     'due:-1day'))
        cls.t(('add', 'due_earlier_today', 'due:today'))
        cls.t(('add', 'due_later_today',   'due:tomorrow'))
        cls.t(('add', 'due_three_days',    'due:3days'))
        cls.t(('add', 'due_next_month',    'due:1month'))
        cls.t(('add', 'due_next_year',     'due:1year'))

    def test_overdue(self):
        """+OVERDUE"""
        code, out, err = self.t(("+OVERDUE", "count"))
        self.assertEqual(out, "3\n", "+OVERDUE == 3 tasks")

    def test_yesterday(self):
        """+YESTERDAY"""
        code, out, err = self.t(("+YESTERDAY", "count"))
        self.assertEqual(out, "1\n", "+YESTERDAY == 1 task")

    def test_due(self):
        """+DUE"""
        code, out, err = self.t(("+DUE", "count"))
        self.assertEqual(out, "3\n", "+DUE == 3 task")

    def test_today(self):
        """+TODAY"""
        code, out, err = self.t(("+TODAY", "count"))
        self.assertEqual(out, "1\n", "+TODAY == 1 task")

    def test_duetoday(self):
        """+DUETODAY"""
        code, out, err = self.t(("+DUETODAY", "count"))
        self.assertEqual(out, "1\n", "+DUETODAY == 1 task")

    def test_tomorrow(self):
        """+TOMORROW"""
        code, out, err = self.t(("+TOMORROW", "count"))
        self.assertEqual(out, "1\n", "+TOMORROW == 1 task")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
