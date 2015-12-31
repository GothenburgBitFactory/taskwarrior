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
        cls.t("1 delete", input="y\n")
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

        code, out, err = self.t("-YEAR all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertNotIn("maximal", out)
        self.assertIn("blocked", out)
        self.assertNotIn("due_eom", out)

    def test_virtual_tag_MONTH(self):
        """Verify 'MONTH' appears when expected"""
        code, out, err = self.t("+MONTH all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertNotIn("blocked", out)
        self.assertIn("due_eom", out)
        # Ignore maximal, due_eow, which may be a different month.

        code, out, err = self.t("-MONTH all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertIn("blocked", out)
        self.assertNotIn("due_eom", out)
        # Ignore maximal, due_eow, which may be a different month.

    def test_virtual_tag_WEEK(self):
        """Verify 'WEEK' appears when expected"""
        code, out, err = self.t("+WEEK all")
        self.assertNotIn("completed", out)
        self.assertNotIn("deleted", out)
        self.assertNotIn("minimal", out)
        self.assertNotIn("blocked", out)
        # Ignore maximal, due_eom, which may be a different week.
        self.assertIn("due_eow", out)

        code, out, err = self.t("-WEEK all")
        self.assertIn("completed", out)
        self.assertIn("deleted", out)
        self.assertIn("minimal", out)
        self.assertIn("blocked", out)
        # Ignore maximal, due_eom, which may be a different week.
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


class TestVirtualTagUDA(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("uda.animal.type",  "string")
        self.t.config("uda.animal.label", "Animal")
        self.t("add one animal:donkey")
        self.t("add two")

    def test_virtual_tag_UDA(self):
        """Verify 'UDA' appears when expected"""
        code, out, err = self.t("+UDA all")
        self.assertIn("one", out)
        self.assertNotIn("two", out)


class TestVirtualTagORPHAN(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add one rc.uda.animal.type:string rc.uda.animal.label:Animal animal:donkey")
        self.t("add two")

    def test_virtual_tag_ORPHAN(self):
        """Verify 'ORPHAN' appears when expected"""
        code, out, err = self.t("+ORPHAN all")
        self.assertIn("one", out)
        self.assertNotIn("two", out)


class Test285(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()
        cls.t.config("verbose", "nothing")

        #              OVERDUE YESTERDAY DUE TODAY TOMORROW WEEK MONTH YEAR
        # due:-1week      Y       -       -    -       -      ?    ?     ?
        # due:-1day       Y       Y       -    -       -      ?    ?     ?
        # due:today       Y       -       Y    Y       -      ?    ?     ?
        # due:tomorrow    -       -       Y    -       Y      ?    ?     ?
        # due:3days       -       -       Y    -       -      ?    ?     ?
        # due:1month      -       -       -    -       -      -    -     ?
        # due:1year       -       -       -    -       -      -    -     -

        cls.t('add due_last_week     due:-1week')
        cls.t('add due_yesterday     due:-1day')
        cls.t('add due_earlier_today due:today')
        cls.t('add due_later_today   due:tomorrow')
        cls.t('add due_three_days    due:3days')
        cls.t('add due_next_month    due:1month')
        cls.t('add due_next_year     due:1year')

    def test_overdue(self):
        """285: +OVERDUE"""
        code, out, err = self.t("+OVERDUE count")
        self.assertEqual(out, "3\n", "+OVERDUE == 3 tasks")

    def test_yesterday(self):
        """285: +YESTERDAY"""
        code, out, err = self.t("+YESTERDAY count")
        self.assertEqual(out, "1\n", "+YESTERDAY == 1 task")

    def test_due(self):
        """285: +DUE"""
        code, out, err = self.t("+DUE count")
        self.assertEqual(out, "3\n", "+DUE == 3 task")

    def test_today(self):
        """285: +TODAY"""
        code, out, err = self.t("+TODAY count")
        self.assertEqual(out, "1\n", "+TODAY == 1 task")

    def test_duetoday(self):
        """285: +DUETODAY"""
        code, out, err = self.t("+DUETODAY count")
        self.assertEqual(out, "1\n", "+DUETODAY == 1 task")

    def test_tomorrow(self):
        """285: +TOMORROW"""
        code, out, err = self.t("+TOMORROW count")
        self.assertEqual(out, "1\n", "+TOMORROW == 1 task")


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
        self.t("list")  # GC/handleRecurrence

        code, out, err = self.t("rc.verbose:nothing tags")
        self.assertNotIn("t1", out)
        self.assertIn("t2", out)

        code, out, err = self.t("rc.verbose:nothing rc.list.all.tags:yes tags")
        self.assertIn("t1", out)
        self.assertIn("t2", out)


class TestBug1700(TestCase):
    def setUp(self):
        self.t = Task()

    def test_tags_overwrite(self):
        """1700: Verify that 'tags:a,b' overwrites existing tags."""
        self.t("add +tag1 +tag2 one")
        code, out, err = self.t("_get 1.tags")
        self.assertIn("tag1,tag2", out)
        self.assertNotIn("tag3", out)

        self.t("1 modify tags:tag2,tag3")
        code, out, err = self.t("_get 1.tags")
        self.assertNotIn("tag1", out)
        self.assertIn("tag2,tag3", out)

class TestBug818(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_tag_filter_partial_match(self):
        """818: Filtering by tag counter-intuitively uses partial match"""
        self.t("add +hannah +anna one")
        code, out, err = self.t("+anna list")
        self.assertIn("one", out)

        self.t("add +anna +hannah two")
        code, out, err = self.t("+anna list")
        self.assertIn("one", out)

        code, out, err = self.t("+hannah list")
        self.assertIn("two", out)

        self.t("add +hannah three")
        self.t("add +anna four")
        code, out, err = self.t("+anna list")
        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertIn("four", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
