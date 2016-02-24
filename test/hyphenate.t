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


class TestHyphenation(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("defaultwidth", "20")
        self.t.config("detection",    "off")
        self.t.config("verbose",      "nothing")

    def test_hyphenation_on_space(self):
        """Split on space instead of hyphenating"""
        self.t("add AAAAAAAAAA BBBBBBBBBB")
        code, out, err = self.t("ls")
        self.assertIn("1 AAAAAAAAAA\n", out)

    @unittest.expectedFailure
    def test_hyphenation(self):
        """Verify hyphenation in the absence of white space"""
        self.t("add AAAAAAAAAABBBBBBBBBBCCCCCCCCCC")
        code, out, err = self.t("ls")
        self.assertIn(" 1 AAAAAAAAAABBBBBB-\n", out)

class TestBug804(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_hyphenation(self):
        """Verify hyphenation is controllable"""
        self.t.config("print.empty.columns",     "yes")
        self.t.config("report.unittest.labels",  "ID,Project,Pri,Description")
        self.t.config("report.unittest.columns", "id,project,priority,description")
        self.t.config("report.unittest.filter",  "status:pending")

        # Setup: Add a tasks, annotate with long word.
        self.t("add one")
        self.t("1 annotate abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz")

        # List with rc.hyphenate=on.
        code, out, err = self.t("rc.defaultwidth:40 rc.hyphenate:on unittest")
        self.assertIn("vwx-\n", out)
        self.assertIn("tuv-\n", out)

        # List with rc.hyphenate=off.
        code, out, err = self.t("rc.defaultwidth:40 rc.hyphenate:off unittest")
        self.assertIn("vwxy\n", out)
        self.assertIn("uvwx\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
