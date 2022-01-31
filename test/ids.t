#!/usr/bin/env python3
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
import tempfile
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import UUID_REGEXP


class TestIDs(TestCase):
    @classmethod
    def setUpClass(self):
        self.t = Task()

        self.t("add one   +A +B")
        self.t("add two   +A"   )
        self.t("add three +A +B")
        self.t("add four"       )
        self.t("add five  +A +B")

    def test_ids_count_A(self):
        """ids +A"""
        code, out, err = self.t("ids +A")
        self.assertRegex(out, "^1-3 5$")

    def test_ids_count_B(self):
        """ids +B"""
        code, out, err = self.t("ids +B")
        self.assertRegex(out, "^1 3 5$")

    def test_ids_count_A_B(self):
        """ids +A -B"""
        code, out, err = self.t("ids +A -B")
        self.assertRegex(out, "^2$")

    def test_get_ids_count_A(self):
        """_ids +A"""
        code, out, err = self.t("_ids +A")
        self.assertRegex(out, "^1\n2\n3\n5$")

    def test_get_zshids_count_A(self):
        """_zshids +A"""
        code, out, err = self.t("_zshids +A")
        self.assertRegex(out, "^1:one\n2:two\n3:three\n5:five$")

    def test_uuids_count_A(self):
        """uuids +A"""
        code, out, err = self.t("uuids +A")
        self.assertRegex(out, "{0} {0} {0} {0}".format(UUID_REGEXP))

    def test_get_uuids_count_A(self):
        """_uuids +A"""
        code, out, err = self.t("_uuids +A")
        self.assertRegex(out, "{0}\n{0}\n{0}\n{0}".format(UUID_REGEXP))

    def test_get_zshuuids_count_A(self):
        """_zshuuids +A"""
        code, out, err = self.t("_zshuuids +A")
        self.assertRegex(
            out, "{0}:one\n{0}:two\n{0}:three\n{0}:five".format(UUID_REGEXP))

    def test_ids_ranges(self):
        """Verify consecutive IDs are compressed into a range"""
        code, out, err = self.t("1 2 3 4 5 ids")
        self.assertIn("1-5", out)


class TestIDMisParse(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_number_not_an_id(self):
        """Verify that numbers in 'add' are not considered IDs"""
        self.t("add 123")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("123\n", out)

    def test_parse_numbers_as_ids_not_patterns(self):
        """Verify that numbers are parsed as IDs"""
        self.t("add 2 two")    # ID 1
        self.t("add 1 one")    # ID 2
        self.t("add 3 three")  # ID 3

        code, out, err = self.t("2 ls rc.verbose:nothing")
        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)


class TestIDRangeParsing(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def generate_tasks(self, n):
        """Generates n tasks for testing purposes"""
        with tempfile.NamedTemporaryFile(mode='w') as f:
            f.write('\n'.join([f'{{"description": "test task {i+1}"}}' for i in range(n)]))
            f.flush()
            code, out, err = self.t(f'import {f.name}')

    def test_single_digit_range(self):
        """Test that parsing single digit ID range works"""
        self.generate_tasks(20)
        code, out, err = self.t("4-6 count")
        self.assertEqual("3", out.strip())

    def test_double_digit_range(self):
        """Test that parsing double digit ID range works"""
        self.generate_tasks(30)
        code, out, err = self.t("14-26 count")
        self.assertEqual("13", out.strip())

    def test_triple_digit_range(self):
        """Test that parsing triple digit ID range works"""
        self.generate_tasks(150)
        code, out, err = self.t("110-128 count")
        self.assertEqual("19", out.strip())

    def test_quadruple_digit_range(self):
        """Test that parsing four digit ID range works"""
        self.generate_tasks(1200)

        # Full range
        code, out, err = self.t("1100-1189 count")
        self.assertEqual("90", out.strip())

        # Partially existing range, 1200 is the last ID
        code, out, err = self.t("1100-1289 count")
        self.assertEqual("101", out.strip())

        # Also if the range resembles time
        code, out, err = self.t("1100-1140 count")
        self.assertEqual("41", out.strip())


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
