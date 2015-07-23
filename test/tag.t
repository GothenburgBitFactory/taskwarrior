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


class TestTags(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()

    def setUp(self):
        """Executed before each test in the class"""

    def test_tag_manipulation(self):
        """Test addition and removal of tags"""
        self.t("add +one This +two is a test +three")
        code, out, err = self.t("_get 1.tags")
        self.assertEqual("one,two,three\n", out)

       # Remove tags.
        self.t("1 modify -three -two -one")
        code, out, err = self.t("_get 1.tags")
        self.assertEqual("\n", out)

        # Add tags.
        self.t("1 modify +four +five +six")
        code, out, err = self.t("_get 1.tags")
        self.assertEqual("four,five,six\n", out)

        # Remove tags.
        self.t("1 modify -four -five -six")
        code, out, err = self.t("_get 1.tags")
        self.assertEqual("\n", out)

        # Add and remove tags.
        self.t("1 modify +duplicate -duplicate")
        code, out, err = self.t("_get 1.tags")
        self.assertEqual("\n", out)

        # Remove missing tag.
        code, out, err = self.t("1 modify -missing")
        self.assertIn("Modified 0 tasks", out)

class TestVirtualTags(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")
        cls.t("log completed")
        cls.t("add deleted")
        cls.t("1 delete")
        cls.t("add minimal")
        cls.t("add maximal +tag pro:PRO pri:H due:yesterday")
        cls.t("3 start")
        cls.t("3 annotate note")
        cls.t("add blocked depends:2")
        cls.t("add due_eom due:eom")
        cls.t("add due_eow due:eow")

    def setUp(self):
        """Executed before each test in the class"""

    def test_virtual_tag_COMPLETED(self):
        """Verify 'COMPLETED' appears when expected"""
        code, out, err = self.t("+COMPLETED all")
        self.assertIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-COMPLETED all")
        self.assertNotIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

    def test_virtual_tag_DELETED(self):
        """Verify 'DELETED' appears when expected"""
        code, out, err = self.t("+DELETED all")
        self.assertNotIn("completed", out)
        self.assertIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-DELETED all")
        self.assertIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

    def test_virtual_tag_PENDING(self):
        """Verify 'PENDING' appears when expected"""
        code, out, err = self.t("+PENDING all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

        code, out, err = self.t("-PENDING all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

    def test_virtual_tag_TAGGED(self):
        """Verify 'TAGGED' appears when expected"""
        code, out, err = self.t("+TAGGED all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-TAGGED all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

    def test_virtual_tag_OVERDUE(self):
        """Verify 'OVERDUE' appears when expected"""
        code, out, err = self.t("+OVERDUE all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-OVERDUE all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

    def test_virtual_tag_BLOCKED(self):
        """Verify 'BLOCKED' appears when expected"""
        code, out, err = self.t("+BLOCKED all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-BLOCKED all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

    def test_virtual_tag_BLOCKING(self):
        """Verify 'BLOCKING' appears when expected"""
        code, out, err = self.t("+BLOCKING all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-BLOCKING all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

    def test_virtual_tag_UNBLOCKED(self):
        """Verify 'UNBLOCKED' appears when expected"""
        code, out, err = self.t("+UNBLOCKED all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

        code, out, err = self.t("-UNBLOCKED all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

    def test_virtual_tag_YEAR(self):
        """Verify 'YEAR' appears when expected"""
        code, out, err = self.t("+YEAR all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

        code, out, err = self.t("-YEAR all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

    def test_virtual_tag_MONTH(self):
        """Verify 'MONTH' appears when expected"""
        code, out, err = self.t("+MONTH all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

        code, out, err = self.t("-MONTH all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

    def test_virtual_tag_WEEK(self):
        """Verify 'WEEK' appears when expected"""
        code, out, err = self.t("+WEEK all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertIn("due_eow", out)

        code, out, err = self.t("-WEEK all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertNotIn("due_eow", out)

    def test_virtual_tag_ACTIVE(self):
        """Verify 'ACTIVE' appears when expected"""
        code, out, err = self.t("+ACTIVE all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-ACTIVE all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)

    def test_virtual_tag_ANNOTATED(self):
        """Verify 'ANNOTATED' appears when expected"""
        code, out, err = self.t("+ANNOTATED all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertIn("maximal", out)
        self.assertNotIn("blocked", out)
        self.assertNotIn("due_eom", out)
        self.assertNotIn("due_eow", out)

        code, out, err = self.t("-ANNOTATED all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertIn("due_eom", out)
        self.assertIn("due_eow", out)


    def test_virtual_tags_helper(self):
        """Verify '_tags' shows appropriate tags"""
        code, out, err = self.t("_tags")
        self.assertIn("PENDING", out)
        self.assertIn("next", out)
        self.assertIn("nocal", out)
        self.assertIn("nocolor", out)
        self.assertIn("nonag", out)
        self.assertIn("tag", out)

class TestDuplicateTags(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")

    def setUp(self):
        """Executed before each test in the class"""

    def test_duplicate_tags(self):
        """When using the 'tags' attribute directly, make sure it strips duplicates"""
        self.t("add one tags:A,A,B,C,C,C")
        code, out, err = self.t("_get 1.tags")
        self.assertEqual("A,B,C\n", out)


class TestListAllTags(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_list_all_tags(self):
        """Verify the 'tags' command obeys 'rc.list.all.tags'

           Create a data set of two tasks, with unique tags, one
           pending, one completed.
        """
        self.t("add +t1 one")
        self.t("add +t2 two")
        self.t("1 done")
        self.t("list") # GC/handleRecurrence

        code, out, err = self.t("rc.verbose:nothing tags")
        self.assertNotIn("t1", out)
        self.assertIn("t2", out)

        code, out, err = self.t("rc.verbose:nothing rc.list.all.tags:yes tags")
        self.assertIn("t1", out)
        self.assertIn("t2", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
