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


class TestAbbreviation(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("abbreviation.minimum", "1")

        self.t("add project:home priority:H hasattributes")
        self.t("add noattributes")

    def verify_attribute(self, expr):
        code, out, err = self.t("list {0}".format(expr))

        self.assertIn("hasattributes", out)
        self.assertNotIn("noattributes", out)

    def test_attribute_abbreviations(self):
        "Test project attribute abbrevations"

        self.verify_attribute("project:home")
        self.verify_attribute("projec:home")
        self.verify_attribute("proje:home")
        self.verify_attribute("proj:home")
        self.verify_attribute("pro:home")

    def test_uda_abbreviations(self):
        "Test uda attribute abbrevations"
        # NOTE This will be a UDA when TW-1541 is closed, for now it is just
        #      one more attribute

        self.verify_attribute("priority:H")
        self.verify_attribute("priorit:H")
        self.verify_attribute("priori:H")
        self.verify_attribute("prior:H")
        self.verify_attribute("prio:H")
        self.verify_attribute("pri:H")

    def verify_command(self, cmd):
        code, out, err = self.t(cmd)

        self.assertIn("MIT license", out)

    def test_command_abbreviations(self):
        "Test version command abbrevations"

        self.verify_command("version")
        self.verify_command("versio")
        self.verify_command("versi")
        self.verify_command("vers")
        self.verify_command("ver")
        self.verify_command("ve")
        self.verify_command("v")


class TestBug1006(TestCase):
    """Bug with expansion of abbreviation "des" in task descriptions and annotations.
       It happens for all the shortcuts for column attributes that are automatically
       completed. This is because DOM elements are checked before standard words
       when strings are tokenized.
    """
    def setUp(self):
        self.t = Task()
        self.t.config("verbose", "affected")

    def initial_tasks(self):
        self.t("add des")
        self.t("1 annotate des")

    def test_completion_of_des_inactive(self):
        "1006: Check that the completion is inactive in task descriptions"

        self.initial_tasks()
        code, out, err = self.t("1 info")

        expected = "Description +des\n"
        errormsg = "Attribute not completed in description"
        self.assertRegexpMatches(out, expected, msg=errormsg)

        notexpected = "description"
        self.assertNotIn(notexpected, out, msg=errormsg)

    def test_completion_as_expected(self):
        "1006: Check that the completion works when needed"

        self.initial_tasks()
        code, out, err = self.t("des:des")

        errormsg = "Task found using its description"
        self.assertIn("1 task", out, msg=errormsg)

    def test_accented_chars(self):
        "1006: Check that é in entrée remains untouched"

        self.t("add entrée interdite")
        code, out, err = self.t("list interdite")

        errormsg = "'entrée' left intact"
        self.assertIn("entrée interdite", out, msg=errormsg)


class TestBug1687(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_som(self):
        """1687: The named date 'som' should take precedence over 'someday', for an exact match"""
        self.t("rc.abbreviation.minimum=2 add one due:som")
        code, out, err = self.t("_get 1.due.year")
        self.assertNotEqual("2038\n", out)

        self.t("rc.abbreviation.minimum=3 add two due:som")
        code, out, err = self.t("_get 2.due.year")
        self.assertNotEqual("2038\n", out)

        self.t("rc.abbreviation.minimum=4 add three due:som")
        code, out, err = self.t("_get 3.due.year")
        self.assertNotEqual("2038\n", out)

        self.t("rc.abbreviation.minimum=4 add three due:some")
        code, out, err = self.t("_get 4.due.year")
        self.assertEqual("2038\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
