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


class TestCustomReports(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("report.foo.description", "DESC")
        self.t.config("report.foo.labels",      "ID,DESCRIPTION")
        self.t.config("report.foo.columns",     "id,description")
        self.t.config("report.foo.sort",        "id+")
        self.t.config("report.foo.filter",      "project:A")

    def test_custom_report_help(self):
        """Verify custom report description is shown in help"""
        code, out, err = self.t("help")
        self.assertRegexpMatches(out, "task <filter> foo\s+DESC\n")

    def test_custom_filter(self):
        """Verify custome report filtr is applied"""
        self.t("add project:A one")
        self.t("add two")
        code, out, err = self.t("foo")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

    def test_custom_labels(self):
        """Verify that custom labels are used in reports"""
        self.t("add one project:A")
        code, out, err = self.t("foo")
        self.assertIn("ID", out)
        self.assertIn("DESCRIPTION", out)

    def test_custom_alternate(self):
        """Verify that color.alternate is used"""
        self.t("add zero")
        self.t("add one project:A")
        self.t.config("color.alternate", "on blue")
        code, out, err = self.t("foo rc._forcecolor:on rc.report.foo.filter:")
        self.assertIn("[44m", out)

class TestCustomErrorHandling(TestCase):
    def setUp(self):
        self.t = Task()

    def test_size_mismatch(self):
        self.t.config("report.foo.columns", "id,description")
        self.t.config("report.foo.labels",  "id")
        code, out, err = self.t.runError("foo")
        self.assertIn("There are different numbers of columns and labels for report 'foo'.", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
