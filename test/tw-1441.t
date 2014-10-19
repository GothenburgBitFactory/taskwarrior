#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug1441(TestCase):
    def setUp(self):
        self.t = Task()

    def test_import_filename(self):
        """import fails if file doesn't exist"""
        command = ("import", "xxx_doesnotexist")
        code, out, err = self.t.runError(command, merge_streams=False)

        self.assertIn("File 'xxx_doesnotexist' not found.", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
