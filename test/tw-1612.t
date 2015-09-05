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


class TestBug1612(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_spurious_whitespace(self):
        """ensure that extra whitespace does not get added.

           tw-1612: Spurious whitespace added in task descriptions around certain symbols
        """
        self.t("add 'foo-bar (http://baz.org/)'")
        self.t("add 'spam (foo bar)'")
        self.t("add '- (foo bar)'")
        self.t("add 'a - (foo bar)'")
        self.t("add '(bar) a / (foo bar)'")

        code, out, err = self.t("_get 1.description")
        self.assertEqual("foo-bar (http://baz.org/)\n", out)

        code, out, err = self.t("_get 2.description")
        self.assertEqual("spam (foo bar)\n", out)

        code, out, err = self.t("_get 3.description")
        self.assertEqual("- (foo bar)\n", out)

        code, out, err = self.t("_get 4.description")
        self.assertEqual("a - (foo bar)\n", out)

        code, out, err = self.t("_get 5.description")
        self.assertEqual("(bar) a / (foo bar)\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
