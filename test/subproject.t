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


class TestSubprojects(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t("add project:abc abc")
        cls.t("add project:ab ab")
        cls.t("add project:a a")
        cls.t("add project:b b")

    def test_project_exact1(self):
        """Verify single character exact"""
        code, out, err = self.t("list project:b")
        self.assertRegexpMatches(out, r"\bb\s")

    def test_project_top1(self):
        """Verify single character parent"""
        code, out, err = self.t("list project:a")
        self.assertRegexpMatches(out, r"\babc\s")
        self.assertRegexpMatches(out, r"\bab\s")
        self.assertRegexpMatches(out, r"\ba\s")

    def test_project_top2(self):
        """Verify double character parent"""
        code, out, err = self.t("list project:ab")
        self.assertRegexpMatches(out, r"\babc\s")
        self.assertRegexpMatches(out, r"\bab\s")

    def test_project_exact3(self):
        """Verify triple character exact"""
        code, out, err = self.t("list project:abc")
        self.assertRegexpMatches(out, r"\babc\s")

    def test_project_mismatch4(self):
        """Verify quad character mismatch"""
        code, out, err = self.t.runError("list project:abcd")
        self.assertIn("No matches", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
