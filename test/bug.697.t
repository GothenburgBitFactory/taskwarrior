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


class TestBug697(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    @unittest.expectedFailure
    def test_blocking_to_recurring(self):
        """Verify that making a blocking task into a recurring task breaks dependencies

           Bug 697: Making a blocking task recur breaks dependency.
             1. Create 2 tasks: "foo" and "bar".
             2. Give "bar" a due date.
             3. Make "foo" depend on "bar".
             4. Make "bar" recur yearly.
        """
        self.t("add one")
        self.t("add two")
        self.t("2 modify due:eom")
        self.t("1 modify depends:2")
        self.t("2 modify recur:yearly")
        self.t("list") # GC/handleRecurrence

        # The problem is that although 1 --> 2, 2 is now a recurring parent, and as 1
        # depends on the parent UUID, it is not something transferred to the child on
        # generation, because the dependency belongs with 1, not 2.

        code, out, err = self.t("_get 1.tag.BLOCKED")
        self.assertEqual("BLOCKED\n", out)

        code, out, err = self.t("_get 2.tag.BLOCKING")
        self.assertEqual("BLOCKING\n", out)

        code, out, err = self.t("_get 3.tag.BLOCKED")
        self.assertEqual("BLOCKED\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
