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


class TestOverride(TestCase):

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("regex",   "off")
        self.t.config("verbose", "nothing")

    def test_override(self):
        """Verify override is displayed in 'show' command"""
        code, out, err = self.t("show regex")
        self.assertRegexpMatches(out, r"regex +off")

        code, out, err = self.t("rc.regex:on show regex")
        self.assertRegexpMatches(out, r"regex +on")


class TestRCSegfault(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_rchyphen_before(self):
        "rc.hyphenated before"
        # This segfaults, ...
        code, our, err = self.t("rc.foo-bar:1 add Sample1")
        self.assertEqual(code, 0)

    def test_rchyphen_after(self):
        "rc.hyphenated after"
        # ... but this passes.
        code, our, err = self.t("add Sample1 rc.foo-bar:1")
        self.assertEqual(code, 0)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
