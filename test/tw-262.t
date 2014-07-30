#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

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
        command = (" project.isnt:tw",)
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
