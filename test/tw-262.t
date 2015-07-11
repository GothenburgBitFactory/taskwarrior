#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

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
        command = ("add", "proj:tw", cls.absent)
        cls.t(command)

        cls.present = "Another project"
        command = ("add", "proj:something_else", cls.present)
        cls.t(command)

    def _check_expectation(self, command):
        code, out, err = self.t(command)

        self.assertIn(self.present, out)
        self.assertNotIn(self.absent, out)

    def test_proj_isnt(self):
        """project.isnt works"""
        command = ("project.isnt:tw",)
        self._check_expectation(command)

    def test_proj_isnt_spaces(self):
        """project.isnt works if wrapped in spaces"""
        command = (" project.isnt:tw ",)
        self._check_expectation(command)

    def test_proj_isnt_space_leading(self):
        """project.isnt works if leading space is present"""
        command = (" project.isnt:tw",)
        self._check_expectation(command)

    def test_proj_isnt_space_trailing(self):
        """project.isnt works if trailing space is present"""
        command = ("project.isnt:tw ",)
        self._check_expectation(command)

    def test_proj_isnt_parenthesis(self):
        """project.isnt works within parenthesis"""
        command = ("(project.isnt:tw)",)
        self._check_expectation(command)

    def test_proj_isnt_parenthesis_space_leading(self):
        """project.isnt works within parenthesis after a leading space"""
        command = (" (project.isnt:tw)",)
        self._check_expectation(command)

    def test_proj_isnt_parenthesis_space_leading_double(self):
        """project.isnt works within parenthesis after a double leading space
        """
        command = (" ( project.isnt:tw)",)
        self._check_expectation(command)

    def test_proj_isnt_parenthesis_space_trailing(self):
        """project.isnt works within parenthesis after a trailing space"""
        command = ("(project.isnt:tw) ",)
        self._check_expectation(command)

    def test_proj_isnt_parenthesis_space_trailing_double(self):
        """project.isnt works within parenthesis after a double trailing space
        """
        command = ("(project.isnt:tw ) ",)
        self._check_expectation(command)

    def test_proj_isnt_spaces_parenthesis(self):
        """project.isnt works within parenthesis and spaces"""
        command = (" (project.isnt:tw) ",)
        self._check_expectation(command)

    def test_proj_isnt_spaces_parenthesis_double(self):
        """project.isnt works within parenthesis and double spaces"""
        command = (" ( project.isnt:tw ) ",)
        self._check_expectation(command)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
