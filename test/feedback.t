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


# Feature 1013: output error, header, footnote and debug messages on standard error
class TestFeature1013(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_errors(self):
        """Verify that errors are sent to standard error"""
        code, out, err = self.t.runError("add")
        self.assertNotIn("Additional text must be provided", out)
        self.assertIn("Additional text must be provided", err)

    def test_headers(self):
        """Verify that headers are sent to standard error"""
        code, out, err = self.t.runError("list")
        self.assertNotIn("TASKRC override:", out)
        self.assertIn("TASKRC override:", err)

    def test_footnotes(self):
        """Verify that footnotes are sent to standard error"""
        self.t("add foo")
        code, out, err = self.t("list rc.foo:bar")
        self.assertNotIn("Perf task", out)
        self.assertNotIn("Configuration override rc.foo:bar", out)
        self.assertIn("Configuration override rc.foo:bar", err)

    def test_debug(self):
        """Verify that debug messages are sent to standard error"""
        code, out, err = self.t.runError("list rc.debug:on")
        self.assertNotIn("Perf task", out)
        self.assertIn("Perf task", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
