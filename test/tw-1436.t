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

from basetest import Task, TestCase, utils


class TestBug1436(TestCase):
    def setUp(self):
        self.t = Task()

    def test_parser_hangs_with_slashes(self):
        """Parser hangs with backslashes"""

        # Yes, seven:
        #   Python turns \\ --> \, therefore \\\\\\\o/ --> \\\\o/
        #   Some process launch thing does the same, therefore \\\\o/ --> \\o/
        #   Taskwarrior sees \\o/, which means \o/
        code, out, err = self.t("add Cheer everyone up \\\\\\\o/")
        self.assertIn("Created task 1", out)

        code, out, err = self.t("_get 1.description")
        self.assertEqual("Cheer everyone up \\o/\n", out)

    def test_parser_ending_escape_slash(self):
        """Task created but not found with ending backslash"""

        # Yes, eight:
        #   Python turns \\ --> \, therefore \\\\\\\\ --> \\\\
        #   Some process launch thing does the same, therefore \\\\ --> \\
        #   Taskwarrior sees \\, which means \
        code, out, err = self.t("add Use this backslash \\\\\\\\")
        self.assertIn("Created task 1", out)

        code, out, err = self.t("list")
        self.assertIn("Use this backslash \\", out)

    def test_backslashes(self):
        """Prove to the reader that backslashes are eaten twice (which means
           \\ --> \) once by Python, and once more by some mystery process
           launch thing.

           This problem is entirely testing artifact, and not Taskwarrior.
        """
        self.echo = Task(taskw=utils.binary_location("/bin/echo"))

        code, out, err = self.echo("xxx \\\\\\\\yyy zzz")       # Shows as 'xxx \\yyy zzz'
        self.tap(out)
        code, out, err = self.echo("xxx \\\\yyy zzz")           # Shows as 'xxx \yyy zzz'
        self.tap(out)
        code, out, err = self.echo("xxx \\yyy zzz")             # Shows as 'xxx yyy zzz'
        self.tap(out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
