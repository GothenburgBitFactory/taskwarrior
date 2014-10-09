#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase


class Test1418(TestCase):
    def setUp(self):
        self.t = Task()

    def test_slash_in_description(self):
        """Check that you can search with a slash (/)"""
        command = ("add", "foo/bar")
        code, out, err = self.t(command)

        command = ("foo/bar",)
        code, out, err = self.t(command)
        self.assertIn("foo/bar", out)

    def test_minus_in_description(self):
        """Check that you can search with a minus (-)"""
        command = ("add", "foo-")
        code, out, err = self.t(command)

        command = ("foo-",)
        code, out, err = self.t(command)
        self.assertIn("foo-", out)

    def test_plus_in_description(self):
        """Check that you can search with a plus (+)"""
        command = ("add", "foo+")
        code, out, err = self.t(command)

        command = ("foo+",)
        code, out, err = self.t(command)
        self.assertIn("foo+", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
