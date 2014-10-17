#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug1436(TestCase):
    def setUp(self):
        self.t = Task()

    def test_parser_hangs_with_slashes(self):
        """Parser hangs with backslashes"""
        expected = "Cheer everyone up \o/"
        code, out, err = self.t(("add", expected))
        self.assertIn("Created task 1", out)

        code, out, err = self.t(("list",))
        self.assertIn(expected, out)

    def test_parser_ending_escape_slash(self):
        """Task created but not found with ending backslash"""
        code, out, err = self.t(("add", "Use this backslash \\\\"))
        self.assertIn("Created task 1", out)

        code, out, err = self.t(("list",))
        self.assertIn("Use this backslash \\", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
