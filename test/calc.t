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
import signal
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import BIN_PREFIX, run_cmd_wait, run_cmd_wait_nofail

CALC = os.path.join(BIN_PREFIX, "calc")


@unittest.skipIf(not os.path.isfile(CALC),
                 "calc binary not available in {0}".format(CALC))
class TestCalc(TestCase):
    def test_regular_math(self):
        """regular math"""
        code, out, err = run_cmd_wait((CALC, "--debug", "12 * 3600 + 34 * 60 + 56"))

        self.assertIn("Eval literal number ↑'12'", out)
        self.assertIn("Eval literal number ↑'3600'", out)
        self.assertIn("Eval literal number ↑'60'", out)
        self.assertIn("Eval literal number ↑'56'", out)
        self.assertRegexpMatches(out, re.compile("^45296$", re.MULTILINE))
        self.assertNotIn("Error", out)
        self.assertNotIn("Error", err)

    def test_postfix_math(self):
        """postfix math"""
        code, out, err = run_cmd_wait((CALC, "--debug", "--postfix", "12 3600 * 34 60 * 56 + +"))

        self.assertIn("Eval literal number ↑'12'", out)
        self.assertIn("Eval literal number ↑'3600'", out)
        self.assertIn("Eval literal number ↑'60'", out)
        self.assertIn("Eval literal number ↑'56'", out)
        self.assertRegexpMatches(out, re.compile("^45296$", re.MULTILINE))
        self.assertNotIn("Error", out)
        self.assertNotIn("Error", err)

    def test_negative_numbers(self):
        """regular math with negative numbers"""
        code, out, err = run_cmd_wait((CALC, "--debug", "2- -3"))

        self.assertIn("Eval literal number ↑'2'", out)
        self.assertIn("Eval _neg_ ↓'3' → ↑'-3'", out)
        self.assertIn("Eval literal number ↑'2'", out)
        self.assertRegexpMatches(out, re.compile("^5$", re.MULTILINE))
        self.assertNotIn("Error", out)
        self.assertNotIn("Error", err)

    def test_help(self):
        """help"""
        code, out, err = run_cmd_wait_nofail((CALC, "--help"))

        self.assertIn("Usage:", out)
        self.assertIn("Options:", out)
        self.assertGreaterEqual(code, 1)

    def test_version(self):
        """version"""
        code, out, err = run_cmd_wait_nofail((CALC, "--version"))

        self.assertRegexpMatches(out, "calc \d\.\d+\.\d+")
        self.assertIn("Copyright", out)
        self.assertGreaterEqual(code, 1)

    def test_duration(self):
        """'15min' is seen as '15', 'min', not '15min' duration"""
        code, out, err = run_cmd_wait((CALC, "--debug", "15min"))

        self.assertNotIn("token infix '15' Date", out)
        self.assertNotIn("token infix 'min' Identifier", out)
        self.assertNotIn("Error: Unexpected stack size: 2", out)
        self.assertNotIn("Error: Unexpected stack size: 2", err)
        self.assertIn("Eval literal duration ↑'PT15M'", out)
        self.assertRegexpMatches(out, re.compile("^PT15M$", re.MULTILINE))


class TestBug1254(TestCase):
    def setUp(self):
        self.t = Task()

    def run_command(self, args):
        code, out, err = self.t(args)

        # We should not see a segmentation fault
        # (negative exit code == 128 - real_exit_code)
        expected = -signal.SIGSEGV
        self.assertNotEqual(expected, code, "Task segfaulted")

        # Instead we expect a clean exit
        expected = 0
        self.assertEqual(expected, code, "Exit code was non-zero ({0})".format(code))

    def test_no_segmentation_fault_calc_negative_multiplication(self):
        """1254: calc can multiply zero and negative numbers
        """
        self.run_command("calc 0*-1")

    def test_calc_positive_multiplication(self):
        """1254: calc can multiply negative zero and positive
        """
        self.run_command("calc 0*1")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
