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

from basetest import Task, TestCase, utils


class TestQuoting(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        # Used to initialize objects that should be re-initialized or
        # re-created for each individual test
        self.t = Task()

    def test_quoted_args_remain_intact(self):
        """Quoted arguments should remain unmolested."""
        self.t("add 'a/b'")
        code, out, err = self.t("_get 1.description")
        self.assertIn("a/b", out)

        self.t("add '1-2'")
        code, out, err = self.t("_get 2.description")
        self.assertIn("1-2", out)


class TestBug268(TestCase):
    def setUp(self):
        self.t = Task()

    def test_add_hyphenated(self):
        """escaped backslashes do not work with 'modify'"""

        self.t("add a b or c")
        self.t('1 modify "/a b/a\/b/"')

        code, out, err = self.t("1 info")
        self.assertIn("a/b or c", out)


class TestBug879(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_backslash_at_eol(self):
        """879: Backslash at end of description/annotation causes problems"""
        self.t("add one\\\\\\\\")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("one\\\n", out)

        self.t("1 annotate two\\\\\\\\")
        code, out, err = self.t("long rc.verbose:nothing")
        self.assertIn("one\\\n", out)
        self.assertIn("two\\\n", out)


class TestBug1436(TestCase):
    def setUp(self):
        self.t = Task()

    def test_parser_hangs_with_slashes(self):
        """1436: Parser hangs with backslashes"""

        # Yes, seven:
        #   Python turns \\ --> \, therefore \\\\\\\o/ --> \\\\o/
        #   Some process launch thing does the same, therefore \\\\o/ --> \\o/
        #   Taskwarrior sees \\o/, which means \o/
        code, out, err = self.t("add Cheer everyone up \\\\\\\o/")
        self.assertIn("Created task 1", out)

        code, out, err = self.t("_get 1.description")
        self.assertEqual("Cheer everyone up \\o/\n", out)

    def test_parser_ending_escape_slash(self):
        """1436: Task created but not found with ending backslash"""

        # Yes, eight:
        #   Python turns \\ --> \, therefore \\\\\\\\ --> \\\\
        #   Some process launch thing does the same, therefore \\\\ --> \\
        #   Taskwarrior sees \\, which means \
        code, out, err = self.t("add Use this backslash \\\\\\\\")
        self.assertIn("Created task 1", out)

        code, out, err = self.t("list")
        self.assertIn("Use this backslash \\", out)

    def test_backslashes(self):
        """1436: Prove to the reader that backslashes are eaten twice (which means
           \\ --> \) once by Python, and once more by some mystery process
           launch thing.

           This problem is entirely testing artifact, and not Taskwarrior.
        """
        self.echo = Task(taskw=utils.binary_location("/bin/echo"))

        code, out, err = self.echo("xxx \\\\\\\\yyy zzz")       # Shows as 'xxx \\yyy zzz'
        code, out, err = self.echo("xxx \\\\yyy zzz")           # Shows as 'xxx \yyy zzz'
        code, out, err = self.echo("xxx \\yyy zzz")             # Shows as 'xxx yyy zzz'


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
