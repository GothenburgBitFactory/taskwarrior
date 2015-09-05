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


class TestBug1006(TestCase):
    """Bug with completion of "des" in task descriptions and annotations. It
    happens for all the shortcuts for column attributes that are automatically
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
        "Check that the completion is inactive in task descriptions"

        self.initial_tasks()
        code, out, err = self.t("1 info")

        expected = "Description +des\n"
        errormsg = "Attribute not completed in description"
        self.assertRegexpMatches(out, expected, msg=errormsg)

        notexpected = "description"
        self.assertNotIn(notexpected, out, msg=errormsg)

    def test_completion_as_expected(self):
        "Check that the completion works when needed"

        self.initial_tasks()
        code, out, err = self.t("des:des")

        errormsg = "Task found using its description"
        self.assertIn("1 task", out, msg=errormsg)

    def test_accented_chars(self):
        "Check that é in entrée remains untouched"

        self.t("add entrée interdite")
        code, out, err = self.t("list interdite")

        errormsg = "'entrée' left intact"
        self.assertIn("entrée interdite", out, msg=errormsg)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
