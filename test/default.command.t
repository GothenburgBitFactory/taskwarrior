#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
from datetime import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestCMD(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()
        cls.t.config("default.command", "list")

        cls.t(('add', 'one'))
        cls.t(('add', 'two'))

    def test_default_command(self):
        """default command"""
        code, out, err = self.t(())
        self.assertIn("task list]", err)

    def test_info_command(self):
        """info command"""
        code, out, err = self.t(('1'))
        self.assertRegexpMatches(out, 'Description\s+one')


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
