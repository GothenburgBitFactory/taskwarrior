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


class TestCMD(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()
        cls.t.config("default.command", "list")

        cls.t('add one')
        cls.t('add two')

    def test_default_command(self):
        """default command"""
        code, out, err = self.t()
        self.assertIn("task list]", err)

    def test_info_command(self):
        """info command"""
        code, out, err = self.t('1')
        self.assertRegexpMatches(out, 'Description\s+one')


class TestDefaults(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("default.command",      "list")
        cls.t.config("default.project",      "PROJECT")
        cls.t.config("uda.priority.default", "M")
        cls.t.config("default.due",          "eom")

    def test_all_defaults(self):
        """Verify all defaults are employed"""
        self.t("add all defaults")
        code, out, err = self.t("export")
        self.assertIn('"description":"all defaults"', out)
        self.assertIn('"project":"PROJECT"', out)
        self.assertIn('"priority":"M"', out)
        self.assertIn('"due":"', out)

    def test_all_specified(self):
        self.t("add project:specific priority:L due:eoy all specified")
        code, out, err = self.t("export")
        self.assertIn('"description":"all specified"', out)
        self.assertIn('"project":"specific"', out)
        self.assertIn('"priority":"L"', out)
        self.assertIn('"due":"', out)

    def test_project_specified(self):
        self.t("add project:specific project specified")
        code, out, err = self.t("export")
        self.assertIn('"description":"project specified"', out)
        self.assertIn('"project":"specific"', out)
        self.assertIn('"priority":"M"', out)
        self.assertIn('"due":"', out)

    def test_priority_specified(self):
        self.t("add priority:L priority specified")
        code, out, err = self.t("export")
        self.assertIn('"description":"priority specified"', out)
        self.assertIn('"project":"PROJECT"', out)
        self.assertIn('"priority":"L"', out)
        self.assertIn('"due":"', out)

    def test_default_command(self):
        self.t("add foo")
        code, out, err = self.t()
        self.assertIn("foo", out)


class TestBug1377(TestCase):
    def setUp(self):
        self.t = Task()

    def test_bad_tag_parser(self):
        """1377: Task doesn't accept tags in default.command"""
        self.t("add Something interesting")
        self.t("add dep:1 NOTSHOWN")
        self.t.config("default.command", "next -BLOCKED")

        code, out, err = self.t()
        self.assertNotIn("NOTSHOWN", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
