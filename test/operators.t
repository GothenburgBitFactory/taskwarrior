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


class TestOperatorsIdentity(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        cls.t("add one   project:A priority:H")
        cls.t("add two   project:A")
        cls.t("add three           priority:H")
        cls.t("add four")

    # AND operator #

    def test_implicit_and(self):
        """operator implicit and :"""
        code, out, err = self.t("ls project:A priority:H")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)

    def test_explicit_and(self):
        """operator explicit and :"""
        code, out, err = self.t("ls project:A and priority:H")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)

    def test_and_not(self):
        """operator and + not :"""
        code, out, err = self.t("ls project:A and priority.not:H")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)

    def test_implicit_and_equal(self):
        """operator implicit and ="""
        code, out, err = self.t("ls project:A priority:H")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)

    def test_explicit_and_equal(self):
        """operator explicit and ="""
        code, out, err = self.t("ls project:A and priority:H")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)

    def test_and_not_equal(self):
        """operator and + not ="""
        code, out, err = self.t("ls project:A and priority.not:H")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)

    # OR operator #

    def test_colon_or_colon(self):
        """operator : or :"""
        code, out, err = self.t("ls project:A or priority:H")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_colon_or_equal(self):
        """operator : or ="""
        code, out, err = self.t("ls project:A or priority=H")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_equal_or_colon(self):
        """operator = or :"""
        code, out, err = self.t("ls project=A or priority:H")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_equal_or_equal(self):
        """operator = or ="""
        code, out, err = self.t("ls project=A or priority=H")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_colon_or_not_colon(self):
        """operator : or not :"""
        code, out, err = self.t("ls project:A or priority.not:H")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertIn("four", out)

    # XOR operator #

    def test_colon_xor_colon(self):
        """operator : xor :"""
        code, out, err = self.t("ls project:A xor priority:H")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_colon_xor_equal(self):
        """operator : xor ="""
        code, out, err = self.t("ls project:A xor priority=H")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_equal_xor_colon(self):
        """operator = xor :"""
        code, out, err = self.t("ls project=A xor priority:H")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_equal_xor_equal(self):
        """operator = xor ="""
        code, out, err = self.t("ls project=A xor priority=H")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)

    def test_colon_xor_not_colon(self):
        """operator : xor not :"""
        code, out, err = self.t("ls project:A xor priority.not:H")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertIn("four", out)


class TestOperatorsQuantity(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        cls.t("add one   due:yesterday priority:H")
        cls.t("add two   due:tomorrow  priority:M")
        cls.t("add three               priority:L")
        cls.t("add four")
        cls.t("add five  due:today")

    # > operator #

    def test_due_after(self):
        """operator due.after:today"""
        code, out, err = self.t("ls due.after:today")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_due_greater(self):
        """operator due > today"""
        code, out, err = self.t("ls due > today")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_priority_above(self):
        """operator priority.above:M"""
        code, out, err = self.t("ls priority.above:M")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_priority_greater(self):
        """operator priority > M"""
        code, out, err = self.t("ls priority > M")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_description_greater(self):
        """operator description > o"""
        code, out, err = self.t("ls description > o")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_urgency_greater(self):
        """operator urgency > 10.0"""
        code, out, err = self.t("ls urgency > 10.0")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    # < operator #

    def test_due_before(self):
        """operator due.before:today"""
        code, out, err = self.t("ls due.before:today")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_due_smaller(self):
        """operator due < today"""
        code, out, err = self.t("ls due < today")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_priority_below(self):
        """operator priority.below:M"""
        code, out, err = self.t("ls priority.below:M")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)

    def test_priority_smaller(self):
        """operator priority < M"""
        code, out, err = self.t("ls priority < M")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)

    def test_description_smaller(self):
        """operator description < o"""
        code, out, err = self.t("ls description < o")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)

    def test_urgency_smaller(self):
        """operator urgency < 10.0"""
        code, out, err = self.t("ls urgency < 10.0")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)

    # >= operator #

    def test_due_greater_equal(self):
        """operator due >= today"""
        code, out, err = self.t("ls due >= today")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)

    def test_priority_greater_equal(self):
        """operator priority >= M"""
        code, out, err = self.t("ls priority >= M")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_description_greater_equal(self):
        """operator description >= o"""
        code, out, err = self.t("ls description >= o")

        # NOTE >= is > + ==, not > + =
        # a single = is a partial match, check: task calc 'a = aa' vs 'a == aa'
        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    def test_urgency_greater_equal(self):
        """operator urgency >= 10.0"""
        code, out, err = self.t("ls urgency >= 10.0")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)

    # <= operator #

    def test_due_smaller_equal(self):
        """operator due <= today"""
        code, out, err = self.t("ls due <= today")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)

    def test_priority_smaller_equal(self):
        """operator priority <= M"""
        code, out, err = self.t("ls priority <= M")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)

    def test_description_smaller_equal(self):
        """operator description <= o"""
        code, out, err = self.t("ls description <= o")

        # NOTE <= is < + ==, not < + =
        # a single = is a partial match, check: task calc 'a = aa' vs 'a == aa'
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)

    def test_urgency_smaller_equal(self):
        """operator urgency <= 10.0"""
        code, out, err = self.t("ls urgency <= 10.0")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
