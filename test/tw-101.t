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
import unittest
import math
# Ensure python finds the local simpletap and basetest modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug101(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

        # Define report with truncated_count style
        self.t.config("report.bug101.columns", "description.truncated_count")

        # Find screen width in order to generate long enough string
        code, out, err = self.t("_get context.width")
        self.width = int(out)
        # Since task strips leading and trailing spaces, for the purposes
        # of these tests, ensure description contains no spaces so we know
        # exactly what string we are expecting
        self.short_description = "A_task_description_"
        # Generate long string
        self.long_description = self.short_description * int(math.ceil(float(self.width)/len(self.short_description)))

    def test_short_no_count(self):
        """Check short description with no annotations"""
        self.t(("add", self.short_description))

        code, out, err = self.t("bug101")

        expected = self.short_description
        self.assertIn(expected, out)

    def test_short_with_count(self):
        """Check short description with annotations"""
        self.t(("add", self.short_description))

        self.t("1 annotate 'A task annotation'")

        code, out, err = self.t("bug101")

        expected = self.short_description + " [1]"
        self.assertIn(expected, out)

    def test_long_no_count(self):
        """Check long description with no annotations"""
        self.t(("add", self.long_description))

        code, out, err = self.t("bug101")

        expected = self.long_description[:(self.width - 3)] + "..."
        self.assertIn(expected, out)

    def test_long_with_count(self):
        """Check long description with annotations"""
        self.t(("add", self.long_description))

        self.t("1 annotate 'A task annotation'")

        code, out, err = self.t("bug101")

        expected = self.long_description[:(self.width - 7)] + "... [1]"
        self.assertIn(expected, out)

    def test_long_with_double_digit_count(self):
        """Check long description with double digit amount of annotations"""
        self.t(("add", self.long_description))

        for i in range(10):
            self.t("1 annotate 'A task annotation'")

        code, out, err = self.t("bug101")

        expected = self.long_description[:(self.width - 8)] + "... [10]"
        self.assertIn(expected, out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
