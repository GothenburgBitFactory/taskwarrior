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


class TestCaseless(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task ()
        cls.t.config("report.ls.columns", "id,project,priority,description")
        cls.t.config("report.ls.labels",  "ID,Proj,Pri,Description")
        cls.t.config("report.ls.sort",    "priority-,project+")
        cls.t.config("report.ls.filter",  "status:pending")

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add one two three")
        self.t("1 annotate four five six")

    def test_description_substitution(self):
        """Verify description substitution with and without case sensitivity"""
        self.t("rc.search.case.sensitive:yes 1 modify /One/ONE/")
        code, out, err = self.t("_get 1.description")
        self.assertNotIn("ONE two three", out)

        self.t("rc.search.case.sensitive:no 1 modify /One/ONE/")
        code, out, err = self.t("_get 1.description")
        self.assertIn("ONE two three", out)

        self.t("rc.search.case.sensitive:yes 1 modify /one/One/")
        code, out, err = self.t("_get 1.description")
        self.assertNotIn("One two three", out)

        self.t("rc.search.case.sensitive:no 1 modify /one/one/")
        code, out, err = self.t("_get 1.description")
        self.assertIn("one two three", out)

    def test_annotation_substitution(self):
        """Verify annotation substitution with and without case sensitivity"""
        self.t("rc.search.case.sensitive:yes 1 modify /Five/FIVE/")
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertNotIn("four FIVE six", out)

        self.t("rc.search.case.sensitive:no 1 modify /Five/FIVE/")
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertIn("four FIVE six", out)

        self.t("rc.search.case.sensitive:yes 1 modify /five/Five/")
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertNotIn("four Five six", out)

        self.t("rc.search.case.sensitive:no 1 modify /five/five/")
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertIn("four five six", out)

    def test_description_filter(self):
        """Verify description filter with and without case sensitivity"""
        code, out, err = self.t.runError("rc.search.case.sensitive:yes ls /One/")
        self.assertNotIn("one two three", out)

        code, out, err = self.t("rc.search.case.sensitive:no ls /One/")
        self.assertIn("one two three", out)

        code, out, err = self.t.runError("rc.search.case.sensitive:yes ls /Five/")
        self.assertNotIn("one two three", out)

        code, out, err = self.t("rc.search.case.sensitive:no ls /Five/")
        self.assertIn("one two three", out)

    def test_annotation_filter(self):
        """Verify annotation filter with and without case sensitivity"""
        code, out, err = self.t.runError("rc.search.case.sensitive:yes ls description.contains:Three")
        self.assertNotIn("one two three", out)

        code, out, err = self.t("rc.search.case.sensitive:no ls description.contains:Three")
        self.assertIn("one two three", out)

        code, out, err = self.t.runError("rc.search.case.sensitive:yes ls description.contains:Six")
        self.assertNotIn("one two three", out)

        code, out, err = self.t("rc.search.case.sensitive:no ls description.contains:Six")
        self.assertIn("one two three", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
