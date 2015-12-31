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
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestRecurrenceProblems(TestCase):
    def setUp(self):
        self.t = Task()

    def test_recurring_due_removal(self):
        """Removing due from a recurring task causes date wrapping"""
        # Originally bug.327.t

        self.t("add foo recur:yearly due:eoy")
        self.t("list")  # Trigger garbage collection

        code, out, err = self.t.runError("2 modify due:")
        self.assertIn("cannot remove the due date from a recurring task", err)

        code, out, err = self.t("list")

        self.assertIn("\n1 task", out)
        self.assertNotIn("1969", out)

    def test_recurring_not_as_epoch(self):
        """Ensure 'until' is rendered as date, not epoch"""
        # Originally bug.368.t

        self.t.config("dateformat.info", "m/d/Y")

        self.t("add foo due:today recur:yearly until:eom")
        code, out, err = self.t("info 1")

        self.assertNotRegexpMatches(out, "Until\s+\d{10}")
        self.assertRegexpMatches(out, "Until\s+\d+\/\d+\/\d{4}")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
