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


class TestPrioritySorting(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")

        cls.t("add H pri:H")
        cls.t("add M pri:M")
        cls.t("add L pri:L")
        cls.t("add _")

    def setUp(self):
        """Executed before each test in the class"""

    def test_priority_sort_under_H(self):
        """Verify priority.under:H works"""
        code, out, err = self.t("priority.under:H ls")
        self.assertNotIn("H", out)
        self.assertIn("M", out)
        self.assertIn("L", out)
        self.assertIn("_", out)

    def test_priority_sort_under_M(self):
        """Verify priority.under:M works"""
        code, out, err = self.t("priority.under:M ls")
        self.assertNotIn("H", out)
        self.assertNotIn("M", out)
        self.assertIn("L", out)
        self.assertIn("_", out)

    def test_priority_sort_under_L(self):
        """Verify priority.under:L works"""
        code, out, err = self.t("priority.under:L ls")
        self.assertNotIn("H", out)
        self.assertNotIn("M", out)
        self.assertNotIn("L", out)
        self.assertIn("_", out)

    def test_priority_sort_under_blank(self):
        """Verify priority.under: works"""
        code, out, err = self.t.runError("priority.under: ls")
        # No output with 'rc.verbose:nothing'

    def test_priority_sort_over_H(self):
        """Verify priority.over:H works"""
        code, out, err = self.t.runError("priority.over:H ls")
        # No output with 'rc.verbose:nothing'

    def test_priority_sort_over_M(self):
        """Verify priority.over:M works"""
        code, out, err = self.t("priority.over:M ls")
        self.assertIn("H", out)
        self.assertNotIn("M", out)
        self.assertNotIn("L", out)
        self.assertNotIn("_", out)

    def test_priority_sort_over_L(self):
        """Verify priority.over:L works"""
        code, out, err = self.t("priority.over:L ls")
        self.assertIn("H", out)
        self.assertIn("M", out)
        self.assertNotIn("L", out)
        self.assertNotIn("_", out)

    def test_priority_sort_over_blank(self):
        """Verify priority.over: works"""
        code, out, err = self.t("priority.over: ls")
        self.assertIn("H", out)
        self.assertIn("M", out)
        self.assertIn("L", out)
        self.assertNotIn("_", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
