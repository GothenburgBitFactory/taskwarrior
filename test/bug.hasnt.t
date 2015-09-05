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


class TestHasHasnt(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_has_hasnt(self):
        """Verify the 'has' and 'hasnt' attribute modifiers"""
        self.t("add foo")              # 1
        self.t("add foo")              # 2
        self.t("2 annotate bar")
        self.t("add foo")              # 3
        self.t("3 annotate bar")
        self.t("3 annotate baz")
        self.t("add bar")              # 4
        self.t("add bar")              # 5
        self.t("5 annotate foo")
        self.t("add bar")              # 6
        self.t("6 annotate foo")
        self.t("6 annotate baz")
        self.t("add one")              # 7
        self.t("7 annotate two")
        self.t("7 annotate three")

        code, out, err = self.t("description.has:foo long")
        self.assertIn("\n 1", out)
        self.assertIn("\n 2", out)
        self.assertIn("\n 3", out)
        self.assertNotIn("\n 4", out)
        self.assertIn("\n 5", out)
        self.assertIn("\n 6", out)
        self.assertNotIn("\n 7", out)

        code, out, err = self.t("description.hasnt:foo long")
        self.assertNotIn("\n 1", out)
        self.assertNotIn("\n 2", out)
        self.assertNotIn("\n 3", out)
        self.assertIn("\n 4", out)
        self.assertNotIn("\n 5", out)
        self.assertNotIn("\n 6", out)
        self.assertIn("\n 7", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
