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


class TestDuplication(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add foo")

    def test_duplication(self):
        """Verify duplicates are the same"""
        self.t("1 duplicate")
        code, out, err = self.t("2 export")
        self.assertIn('"description":"foo"', out)
        self.assertIn('"status":"pending"', out)

    def test_duplication_with_en_passant(self):
        """Verify en-passant changes work with duplication"""
        self.t("1 duplicate priority:H /foo/FOO/ +tag")
        code, out, err = self.t("2 export")
        self.assertIn('"description":"FOO"', out)
        self.assertIn('"status":"pending"', out)
        self.assertIn('"priority":"H"', out)
        self.assertIn('"tags":["tag"]', out)

    def test_duplication_with_no_tasks(self):
        """Verify an empty filter generates an error"""
        code, out, err = self.t.runError("999 duplicate")
        self.assertIn("No tasks specified.", err)

    def test_duplication_showing_uuid(self):
        """Verify duplicate can show uuid"""
        code, out, err = self.t("1 duplicate rc.verbose:new-uuid")
        self.assertRegexpMatches(out, "[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}")


class TestDuplication2(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_duplication_recurrence(self):
        """Verify that recurring tasks are properly duplicated"""
        self.t("add R due:tomorrow recur:weekly")
        self.t("list")   # To force handleRecurrence().

        code, out, err = self.t("1 duplicate")
        self.assertIn("The duplicated task is too", out)

        code, out, err = self.t("2 duplicate")
        self.assertIn("The duplicated task is not", out)

        self.t("list")   # To force handleRecurrence().
        code, out, err = self.t("1 export")
        self.assertIn('"status":"recurring"', out)

        code, out, err = self.t("2 export")
        self.assertIn('"status":"pending"', out)
        self.assertIn('"parent":', out)

        code, out, err = self.t("3 export")
        self.assertIn('"status":"recurring"', out)

        code, out, err = self.t("4 export")
        self.assertIn('"status":"pending"', out)
        self.assertNotIn('"parent":', out)

        code, out, err = self.t("5 export")
        self.assertIn('"status":"pending"', out)
        self.assertIn('"parent":', out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
