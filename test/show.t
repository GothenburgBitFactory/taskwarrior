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


class TestShowCommand(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_show_no_arg(self):
        """Verify show command lists all with no arg provided"""
        code, out, err = self.t("show")
        self.assertIn("dateformat", out)
        self.assertIn("regex", out)

    def test_show_all_arg(self):
        """Verify show command lists all with no arg provided"""
        code, out, err = self.t("show all")
        self.assertIn("dateformat", out)
        self.assertIn("regex", out)

    def test_show_one_arg(self):
        """Verify show command lists one result with an arg provided"""
        self.t.config("default.due", "tomorrow")
        code, out, err = self.t("show default.due")
        self.assertNotIn("default.command", out)
        self.assertIn("default.due", out)
        self.assertNotIn("default.project", out)

    def test_show_error_on_multi_args(self):
        """Verify show command errors on multiple args"""
        code, out, err = self.t.runError("show one two")
        self.assertIn("You can only specify 'all' or a search string.", err)

    def test_show_no_unrecognized(self):
        """Verify show command lists all with no arg provided"""
        self.t.config("foo", "bar")
        code, out, err = self.t("show")
        self.assertIn("Your .taskrc file contains these unrecognized variables:\n  foo", out)


class TestShowHelperCommand(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_show_helper_no_arg(self):
        """Verify _show command lists all with no arg provided"""
        code, out, err = self.t("_show")
        self.assertIn("debug=no\n", out)
        self.assertIn("verbose=", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
