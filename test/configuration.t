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


class TestConfiguration(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_default_config(self):
        """Verify that by default, the 'show' command has no complaints"""
        code, out, err = self.t("show")
        self.assertNotIn("Configuration error:", out)
        self.assertNotIn("unrecognized variables", out)
        self.assertNotIn("unrecognized value", out)
        self.assertNotIn("data.location not specified", out)
        self.assertNotIn("data.location contains", out)

    def test_obsolete_config(self):
        """Verify that the 'show' command detects obsolete configuration"""
        self.t.config("foo", "1")
        code, out, err = self.t("show")
        self.assertIn("unrecognized variables", out)
        self.assertIn("  foo\n", out)

    def test_config_completion(self):
        """verify that the '_config' command generates a full list"""
        code, out, err = self.t("_config")
        self.assertIn("_forcecolor", out) # first
        self.assertIn("xterm.title", out) # last

    def test_config_nothing(self):
        """Verify error handling with no args"""
        code, out, err = self.t.runError("config")
        self.assertIn("Specify the name of a config variable to modify.", err)

    def test_config_no_change(self):
        """Verify error handling with no change"""
        code, out, err = self.t.runError("config foo")
        self.assertIn("No entry named 'foo' found.", err)


class TestBug1475(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_config_unmolested(self):
        """1475: Verify that a config value is not borked by lex/eval"""
        self.t.config("name", "one/two/three")

        code, out, err = self.t("_get rc.name")
        self.assertEqual("one/two/three\n", out)

    def test_config_unmolested_2(self):
        """1475: Verify that a config value is not borked by lex/eval - literal"""
        self.t("config name one/two/three", input="y\n")

        code, out, err = self.t("_get rc.name")
        self.assertEqual("one/two/three\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
