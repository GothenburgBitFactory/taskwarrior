#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
import unittest
from datetime import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestMath(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()
        cls.t.config("verbose", "nothing")
        cls.t.config("dateformat", "YYYY-MM-DD")

        # YYYY-12-21.
        cls.when = "%d-12-21T23:59:59\n" % datetime.now().year

        # Different ways of specifying YYYY-12-21.
        cls.t("add one   due:eoy-10days")
        cls.t("add two   due:'eoy-10days'")
        cls.t("add three 'due:eoy-10days'")
        cls.t("add four  due:'eoy - 10days'")
        cls.t("add five  'due:eoy - 10days'")
        cls.t("add six   'due:%d-12-31T23:59:59 - 10days'" % datetime.now().year)

    def test_compact_unquoted(self):
        """compact unquoted"""
        code, out, err = self.t('_get 1.due')
        self.assertEqual(out, self.when)

    def test_compact_value_quoted(self):
        """compact value quoted"""
        code, out, err = self.t('_get 2.due')
        self.assertEqual(out, self.when)

    def test_compact_arg_quoted(self):
        """compact arg quoted"""
        code, out, err = self.t('_get 3.due')
        self.assertEqual(out, self.when)

    def test_sparse_value_quoted(self):
        """sparse value quoted"""
        code, out, err = self.t('_get 4.due')
        self.assertEqual(out, self.when)

    def test_sparse_arg_quoted(self):
        """sparse arg quoted"""
        code, out, err = self.t('_get 5.due')
        self.assertEqual(out, self.when)

    def test_sparse_arg_quoted_literal(self):
        """sparse arg quoted literal"""
        code, out, err = self.t('_get 6.due')
        self.assertEqual(out, self.when)

class TestBug851(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t('add past due:-2days')
        cls.t('add future due:2days')

    def setUp(self):
        """Executed before each test in the class"""

    def test_attribute_before_with_math(self):
        """851: Test due.before:now+1d"""
        code, out, err = self.t('due.before:now+1day ls')
        self.assertIn("past", out)
        self.assertNotIn("future", out)

    def test_attribute_after_with_math(self):
        """851: Test due.after:now+1d"""
        code, out, err = self.t('due.after:now+1day ls')
        self.assertNotIn("past", out)
        self.assertIn("future", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
