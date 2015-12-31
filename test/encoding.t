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
import re
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestUtf8(TestCase):
    def setUp(self):
        self.t = Task()

    def test_utf8_tags(self):
        """Correct handling of UTF8 characters"""
        self.t("add one +osobní")

        code, out, err = self.t("list +osobní")
        self.assertIn("one", out)

        code, out, err = self.t.runError("list -osobní")
        self.assertNotIn("one", out)

        self.t("add two +föo")
        code, out, err = self.t("list +föo")
        self.assertIn("two", out)
        self.assertNotIn("one", out)

        code, out, err = self.t("list -föo")
        self.assertNotIn("two", out)
        self.assertIn("one", out)

    def test_wide_utf8(self):
        """Text alignment in reports with wide utf8 characters"""
        # Originally Bug #455 - Text alignment in reports is broken when text
        #                       contains wide utf8 characters
        self.t.config("print.empty.columns", "no")

        self.t(("add", "abc", "pro:Bar\u263a"))
        self.t("add def pro:Foo")

        code, out, err = self.t("ls")

        expected = re.compile("\S\s{4}abc", re.MULTILINE)
        self.assertRegexpMatches(out, expected)
        expected = re.compile("\S\s{5}def", re.MULTILINE)
        self.assertRegexpMatches(out, expected)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
