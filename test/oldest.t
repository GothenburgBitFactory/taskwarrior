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
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestOldestAndNewest(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t("add one    entry:2015-07-10T10:30:00")
        cls.t("add two    entry:2015-07-10T10:30:01")
        cls.t("add three  entry:2015-07-10T10:30:02")
        cls.t("add four   entry:2015-07-10T10:30:03")
        cls.t("add five   entry:2015-07-10T10:30:04")
        cls.t("add six    entry:2015-07-10T10:30:05")
        cls.t("add seven  entry:2015-07-10T10:30:06")
        cls.t("add eight  entry:2015-07-10T10:30:07")
        cls.t("add nine   entry:2015-07-10T10:30:08")
        cls.t("add ten    entry:2015-07-10T10:30:09")
        cls.t("add eleven entry:2015-07-10T10:30:10")

    def setUp(self):
        """Executed before each test in the class"""

    def test_oldest_ten(self):
        """Test oldest report + limit:10"""
        code, out, err = self.t("oldest limit:10")
        self.assertIn(" one", out)
        self.assertIn(" two", out)
        self.assertIn(" three", out)
        self.assertIn(" four", out)
        self.assertIn(" five", out)
        self.assertIn(" six", out)
        self.assertIn(" seven", out)
        self.assertIn(" eight", out)
        self.assertIn(" nine", out)
        self.assertIn(" ten", out)
        self.assertNotIn(" eleven", out)

    def test_oldest_three(self):
        """Test oldest report + limit:3"""
        code, out, err = self.t("oldest limit:3")
        self.assertIn(" one", out)
        self.assertIn(" two", out)
        self.assertIn(" three", out)
        self.assertNotIn(" four", out)
        self.assertNotIn(" five", out)
        self.assertNotIn(" six", out)
        self.assertNotIn(" seven", out)
        self.assertNotIn(" eight", out)
        self.assertNotIn(" nine", out)
        self.assertNotIn(" ten", out)
        self.assertNotIn(" eleven", out)

    def test_newest_ten(self):
        """Test newest report + limit:10"""
        code, out, err = self.t("newest limit:10")
        self.assertNotIn(" one", out)
        self.assertIn(" two", out)
        self.assertIn(" three", out)
        self.assertIn(" four", out)
        self.assertIn(" five", out)
        self.assertIn(" six", out)
        self.assertIn(" seven", out)
        self.assertIn(" eight", out)
        self.assertIn(" nine", out)
        self.assertIn(" ten", out)
        self.assertIn(" eleven", out)

    def test_newest_three(self):
        """Test newest report + limit:3"""
        code, out, err = self.t("newest limit:3")
        self.assertNotIn(" one", out)
        self.assertNotIn(" two", out)
        self.assertNotIn(" three", out)
        self.assertNotIn(" four", out)
        self.assertNotIn(" five", out)
        self.assertNotIn(" six", out)
        self.assertNotIn(" seven", out)
        self.assertNotIn(" eight", out)
        self.assertIn(" nine", out)
        self.assertIn(" ten", out)
        self.assertIn(" eleven", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
