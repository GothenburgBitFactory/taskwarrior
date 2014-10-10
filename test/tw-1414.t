#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


class TestBug1414(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("default.command", "exec echo hello")
        self.t.config("verbose", "no")

    def test_execute(self):
        """use execute"""

        code, out, err = self.t(("exec", "echo hello"))

        self.assertIn("hello", out)

    def test_exec_alias(self):
        """use exec in alias"""

        self.t.config("alias.asdf", "exec echo hello")

        code, out, err = self.t(("asdf", ""))

        self.assertIn("hello", out)

    def test_execute_alias(self):
        """use execute in alias"""

        self.t.config("alias.asdf", "execute echo hello")

        code, out, err = self.t(("asdf", ""))

        self.assertIn("hello", out)

    def test_default_command(self):
        """use exec in default.command"""

        code, out, err = self.t()

        self.assertIn("hello", out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
