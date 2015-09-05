#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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


class TestAlias(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_simple_alias_to_project(self):
        """Access a project via aliases"""

        # Setup aliases
        self.t.config("alias.foo", "_projects")
        self.t.config("alias.bar", "foo")
        self.t.config("alias.baz", "bar")
        self.t.config("alias.qux", "baz")

        # Setup a task with dummy project called Home
        expected = "Home"
        self.t("add project:{0} foo".format(expected))

        # Sanity check that _projects command outputs the "Home" project
        code, out, err = self.t("_projects")
        self.assertIn(expected, out,
                      msg="task _projects -> Home")

        # Check that foo command outputs the "Home" project
        code, out, err = self.t("foo")
        self.assertIn(expected, out,
                      msg="task foo -> _projects > Home")

        # Check that bar command outputs the "Home" project
        code, out, err = self.t("bar")
        self.assertIn(expected, out,
                      msg="task bar -> foo > _projects > Home")

        # Check that baz command outputs the "Home" project
        code, out, err = self.t("baz")
        self.assertIn(expected, out,
                      msg="task baz -> bar > foo > _projects > Home")

        # Check that qux command outputs the "Home" project
        code, out, err = self.t("qux")
        self.assertIn(expected, out,
                      msg="task qux -> baz > bar > foo > _projects > Home")

    def test_alias_with_implicit_filter(self):
        """Test alias containing simple filter string"""

        # Setup alias with simple filter string
        self.t.config("alias.foofilter", "project:Home _projects")

        # Setup tasks for projects Home and Work
        self.t("add project:Home Home task")
        self.t("add project:Work Work task")

        # Sanity check that _projects command outputs
        # both the "Home" and "Work" projects
        code, out, err = self.t("_projects")
        self.assertIn("Home", out,
                      msg="task _projects -> Home")
        self.assertIn("Work", out,
                      msg="task _projects -> Work")

        # Check that foo command outputs the "Home" project
        code, out, err = self.t("foofilter")
        self.assertIn("Home", out,
                msg="task foofilter -> project:Home _projects > Home")
        self.assertNotIn("Work", out,
                msg="task foofilter -> project:Home _projects > Work")

    def test_alias_with_implicit_complex_filter(self):
        """Test alias containing filter string with conjuction"""

        # Setup alias with simple filter string
        self.t.config("alias.hometoday", "project:Home and due:today minimal")

        # Setup tasks for projects Home and Work
        self.t("add project:Home due:today Home urgent task")
        self.t("add project:Home Home task")
        self.t("add project:Work due:today Work task")

        # Check that hometoday command outputs the "Home urgent task"
        code, out, err = self.t("hometoday")
        self.assertIn("Home urgent task", out,
                msg="task hometoday -> project:Home and due:today minimal > "
                    "Home urgent task")

        # It should not output "Home task", as that one is not due:today
        self.assertNotIn("Home task", out,
                msg="task hometoday -> project:Home and due:today minimal > "
                    "Home task")

        # It should not output "Work task" either, it has entirely wrong
        # project
        self.assertNotIn("Work task", out,
                msg="task hometoday -> project:Home and due:today minimal > "
                    "Work task")

class TestAliasesCommand(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_aliases_helper(self):
        """Verify that aliases are listed by the _aliases command"""
        self.t.config("alias.foo", "bar")
        code, out, err = self.t("_aliases")
        self.assertIn("foo", out)

class TestBug1652(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add one")

    def test_odd_alias(self):
        """Verify that 'delete' is not lexed further"""
        self.t.config("alias.rm", "delete")
        self.t.config("confirmation", "off")
        code, out, err = self.t("1 rm")
        self.assertIn("Deleted 1 task.", out)
        self.assertNotIn("No matches.", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
