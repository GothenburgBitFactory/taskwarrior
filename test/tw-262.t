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


class TestBug262(TestCase):
    @classmethod
    def setUp(cls):
        cls.t = Task()

        cls.absent = "Taskwarrior project"
        cls.t(("add", "proj:tw", cls.absent))

        cls.present = "Another project"
        cls.t(("add", "proj:something_else", cls.present))

    def _check_expectation(self, command):
        # Add quotes to ensure spaces around are not split by the shell lexer
        code, out, err = self.t((command,))

        self.assertIn(self.present, out)
        self.assertNotIn(self.absent, out)

    def test_proj_isnt(self):
        """project.isnt works"""
        self._check_expectation("project.isnt:tw")

    def test_proj_isnt_spaces(self):
        """project.isnt works if wrapped in spaces"""
        self._check_expectation(" project.isnt:tw ")

    def test_proj_isnt_space_leading(self):
        """project.isnt works if leading space is present"""
        self._check_expectation(" project.isnt:tw")

    def test_proj_isnt_space_trailing(self):
        """project.isnt works if trailing space is present"""
        self._check_expectation("project.isnt:tw ")

    def test_proj_isnt_parenthesis(self):
        """project.isnt works within parenthesis"""
        self._check_expectation("(project.isnt:tw)")

    def test_proj_isnt_parenthesis_space_leading(self):
        """project.isnt works within parenthesis after a leading space"""
        self._check_expectation(" (project.isnt:tw)")

    def test_proj_isnt_parenthesis_space_leading_double(self):
        """project.isnt works within parenthesis after a double leading space
        """
        self._check_expectation(" ( project.isnt:tw)")

    def test_proj_isnt_parenthesis_space_trailing(self):
        """project.isnt works within parenthesis after a trailing space"""
        self._check_expectation("(project.isnt:tw) ")

    def test_proj_isnt_parenthesis_space_trailing_double(self):
        """project.isnt works within parenthesis after a double trailing space
        """
        self._check_expectation("(project.isnt:tw ) ")

    def test_proj_isnt_spaces_parenthesis(self):
        """project.isnt works within parenthesis and spaces"""
        self._check_expectation(" (project.isnt:tw) ")

    def test_proj_isnt_spaces_parenthesis_double(self):
        """project.isnt works within parenthesis and double spaces"""
        self._check_expectation(" ( project.isnt:tw ) ")

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
