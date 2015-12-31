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


class TestAppend(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add foo")

    def test_append(self):
        """Add a task and then append more description"""
        code, out, err = self.t("1 append bar")

        expected = "Appended 1 task."
        self.assertIn(expected, out)

        code, out, err = self.t("info 1")

        expected = "Description\s+foo\sbar\n"
        self.assertRegexpMatches(out, expected)

    def test_append_error_on_empty(self):
        """Should cause an error when nothing is appended"""
        code, out, err = self.t.runError("1 append")

        expected = "Additional text must be provided."
        self.assertIn(expected, err)

        notexpected = "Appended 1 task."
        self.assertNotIn(notexpected, out)


class TestBug440(TestCase):
    # Bug #440: Parser recognizes an attempt to simultaneously subst and
    #           append, but doesn't do it
    def setUp(self):
        self.t = Task()

    def test_subst_and_append_at_once(self):
        """Simultaneous substitution and append"""
        self.t("add Foo")
        self.t("add Foo")

        self.t("1 append /Foo/Bar/ Appendtext")
        self.t("2 append Appendtext /Foo/Bar/")

        code1, out1, err1 = self.t("1 ls")
        code2, out2, err2 = self.t("2 ls")

        self.assertNotIn("Foo", out1)
        self.assertRegexpMatches(out1, "\w+ Appendtext")

        self.assertNotIn("Foo", out2)
        self.assertRegexpMatches(out2, "\w+ Appendtext")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
