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
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestSearch(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("verbose", "nothing")
        self.t("add one")
        self.t("1 annotate anno")
        self.t("add two")

    def test_plain_arg(self):
        """Verify plain args are interpreted as search terms

           tw-1635: Running "task anystringatall" does not filter anything
        """
        code, out, err = self.t("one list")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

    def test_plain_arg_annotation(self):
        """Verify that search works in annotations"""
        code, out, err = self.t("list ann")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

    def test_plain_arg_after_cmd(self):
        """Verify plain args are interpreted as search terms, after the command"""
        code, out, err = self.t("list one")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

class TestBug1472(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")
        cls.t("add A to Z")
        cls.t("add Z to A")

    def setUp(self):
        """Executed before each test in the class"""

    def test_startswith_regex(self):
        """Verify .startswith works with regexes"""
        code, out, err = self.t("rc.regex:on description.startswith:A ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)

    def test_endswith_regex(self):
        """Verify .endswith works with regexes"""
        code, out, err = self.t("rc.regex:on description.endswith:Z ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)

    def test_startswith_no_regex(self):
        """Verify .startswith works without regexes"""
        code, out, err = self.t("rc.regex:off description.startswith:A ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)

    def test_endswith_no_regex(self):
        """Verify .endswith works without regexes"""
        code, out, err = self.t("rc.regex:off description.endswith:Z ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)


# TODO Search with patterns


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
