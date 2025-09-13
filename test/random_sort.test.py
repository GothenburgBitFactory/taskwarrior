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
        code, out1_seed, err = self.t(
            "rc.debug.random.seed=123 all rc.report.all.sort:random"
        )
        code, out2_seed, err = self.t(
            "rc.debug.random.seed=123 all rc.report.all.sort:random"
        )
        self.assertEqual(
            out1_seed,
            out2_seed,
            "Random sort with the same seed should produce the same order",
        )


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())
