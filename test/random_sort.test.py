#!/usr/bin/env python3
###############################################################################
#
# Copyright 2006 - 2025, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
# https://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestRandomSort(TestCase):
    def setUp(self):
        self.t = Task()
        self.t("add one")
        self.t("add two")
        self.t("add three")
        self.t("add four")
        self.t("add five")

    def test_random_sort_deterministic(self):
        """Verify that 'sort:random' with different seeds produces different orderings."""
        # With a fixed seed, the order is always the same.
        code, out1_seed1, err = self.t(
            "rc.debug.random.seed=123 all rc.report.all.sort:random"
        )
        code, out2_seed1, err = self.t(
            "rc.debug.random.seed=123 all rc.report.all.sort:random"
        )
        self.assertEqual(
            out1_seed1,
            out2_seed1,
            "Random sort with the same seed should produce the same order",
        )

        # With a different fixed seed, the order is different.
        code, out1_seed2, err = self.t(
            "rc.debug.random.seed=456 all rc.report.all.sort:random"
        )
        self.assertNotEqual(
            out1_seed1,
            out1_seed2,
            "Random sort with different seeds should produce different orders",
        )

    def test_random_and_id_sort(self):
        """Verify that 'sort:random,id' is not the same as 'sort:id'."""
        code, out_id, err = self.t("all rc.report.all.sort:id")
        code, out_random_id, err = self.t(
            "rc.debug.random.seed=123 all rc.report.all.sort:random,id"
        )
        self.assertNotEqual(
            out_id, out_random_id, "random,id sort should be different from id sort"
        )


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())
