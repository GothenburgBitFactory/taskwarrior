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


class TestSubstitutions(TestCase):

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_substitution(self):
        """Verify substitution for task description"""
        self.t.config("regex", "off")

        self.t("add foo foo foo")
        self.t("1 modify /foo/FOO/")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("FOO foo foo\n", out)

        self.t("1 modify /foo/FOO/g")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("FOO FOO FOO\n", out)

        self.t("1 modify /FOO/aaa/")
        self.t("1 modify /FOO/bbb/")
        self.t("1 modify /FOO/ccc/")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("aaa bbb ccc\n", out)

        self.t("1 modify /bbb//")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("aaa  ccc\n", out)

    def test_substitution_annotation(self):
        """Verify substitution for task annotation"""
        self.t("add foo foo foo")
        self.t("1 annotate bar bar bar")
        self.t("1 modify /bar/BAR/")
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertEqual("BAR bar bar\n", out)

        self.t("1 modify /bar/BAR/g")
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertEqual("BAR BAR BAR\n", out)

    def test_substitution_regex(self):
        """Verify regex substitution for task description"""
        self.t.config("regex", "on")
        self.t("add aaa bbb")
        self.t("1 modify /b{3}/BbB/")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("aaa BbB\n", out)

class TestBug441(TestCase):
    def setUp(self):
        self.t = Task()

    def test_bad_colon_replacement(self):
        """441: Substitution containing a colon"""

        self.t("add one two three")
        self.t("1 modify /two/two:/")

        code, out, err = self.t("ls")
        self.assertIn("one two: three", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
