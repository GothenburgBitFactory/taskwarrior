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
        code, out, err = self.t("exec echo hello")
        self.assertIn("hello", out)

    def test_exec_alias(self):
        """use exec in alias"""
        self.t.config("alias.asdf", "exec echo hello")
        code, out, err = self.t("asdf")
        self.assertIn("hello", out)

    def test_execute_alias(self):
        """use execute in alias"""
        self.t.config("alias.asdf", "execute echo hello")
        code, out, err = self.t("asdf")
        self.assertIn("hello", out)

    def test_default_command(self):
        """use exec in default.command"""
        code, out, err = self.t()
        self.assertIn("hello", out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
