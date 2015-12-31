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
import platform
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestSearch(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("verbose", "nothing")
        self.t("add one")
        self.t("1 annotate anno")
        self.t("add two")

    def test_plain_arg(self):
        """Verify plain args are interpreted as search terms

           tw-1635: Running "task anystringatall" does not filter anything
        """
        code, out, err = self.t("one list")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

    def test_plain_arg_annotation(self):
        """Verify that search works in annotations"""
        code, out, err = self.t("list ann")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

    def test_plain_arg_after_cmd(self):
        """Verify plain args are interpreted as search terms, after the command"""
        code, out, err = self.t("list one")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

class Test1418(TestCase):
    def setUp(self):
        self.t = Task()

    # Helper methods
    def find_in_list(self, description):
        code, out, err = self.t("list")
        self.assertIn(description, out)

    def search_task_pattern(self, description):
        command = "/" + description.replace("/", "\\/") + "/"
        code, out, err = self.t(command)
        self.assertIn(description, out)

    def add_search_task(self, description):
        command = "add " + description
        self.t(command)

    def add_search_task_description(self, description):
        command = "add description:'" + description + "'"
        self.t(command)

    def test_slash_in_description(self):
        """1418: Check that you can search with a slash (/)"""
        description = "foo/"
        self.add_search_task(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_minus_in_description(self):
        """1418: Check that you can search with a minus (-)"""
        description = "foo-"
        self.add_search_task(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_plus_in_description(self):
        """1418: Check that you can search with a plus (+)"""
        description = "foo+"
        self.add_search_task(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_explicit_slash_in_description(self):
        """1418: Can add a task with trailing slash (/) using description:"" """
        description = "foo/"
        self.add_search_task_description(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_explicit_minus_in_description(self):
        """1418: Can add a task with trailing minus (-) using description:"" """
        description = "foo-"
        self.add_search_task_description(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_explicit_plus_in_description(self):
        """1418: Can add a task with trailing plus (+) using description:"" """
        description = "foo+"
        self.add_search_task_description(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_slash_plus_in_description(self):
        """1418: Can add and search a task with (+) in description"""
        description = "foo+"
        self.add_search_task(description)
        self.find_in_list(description)

        # Different from the other tests, because we want to escape the '+'
        # in the regex, but not in the 'add' or 'list'
        code, out, err = self.t("/foo\\+/")
        self.assertIn(description, out)

class TestBug1472(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")
        cls.t("add A to Z")
        cls.t("add Z to A")

    def setUp(self):
        """Executed before each test in the class"""

    def test_startswith_regex(self):
        """1472: Verify .startswith works with regexes"""
        code, out, err = self.t("rc.regex:on description.startswith:A ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)

    def test_endswith_regex(self):
        """1472: Verify .endswith works with regexes"""
        code, out, err = self.t("rc.regex:on description.endswith:Z ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)

    def test_startswith_no_regex(self):
        """1472: Verify .startswith works without regexes"""
        code, out, err = self.t("rc.regex:off description.startswith:A ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)

    def test_endswith_no_regex(self):
        """1472: Verify .endswith works without regexes"""
        code, out, err = self.t("rc.regex:off description.endswith:Z ls")
        self.assertIn("A to Z", out)
        self.assertNotIn("Z to A", out)


class Test1469(TestCase):
    def setUp(self):
        self.t = Task()
        self.t('add foo')
        self.t('add "neue Badmöbel kaufen"')

    def test_implicit_search_sensitive_regex(self):
        """1469: Implicit search, case sensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=on')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_implicit_search_sensitive_noregex(self):
        """1469: Implicit search, case sensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    @unittest.skipIf('CYGWIN' in platform.system(), 'Skipping regex case-insensitive test for Cygwin')
    def test_implicit_search_insensitive_regex(self):
        """1469: Implicit search, case insensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=on')
        self.assertEqual(0, code,
                         "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_implicit_search_insensitive_noregex(self):
        """1469: Implicit search, case insensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_explicit_search_sensitive_regex(self):
        """1469: Explicit search, case sensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=on')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_explicit_search_sensitive_noregex(self):
        """1469: Explicit search, case sensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    @unittest.skipIf('CYGWIN' in platform.system(), 'Skipping regex case-insensitive test for Cygwin')
    def test_explicit_search_insensitive_regex(self):
        """1469: Explicit search, case insensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=on')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_explicit_search_insensitive_noregex(self):
        """1469: Explicit search, case insensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)


class TestBug1479(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_description_with_spaces(self):
        """1479: Verify that a description of 'one two' is searchable"""
        self.t("add project:P1 one")
        self.t("add project:P2 one two")

        code, out, err = self.t("description:one\ two list")
        self.assertNotIn("P1", out)
        self.assertIn("P2", out)

        code, out, err = self.t("description:'one two' list")
        self.assertNotIn("P1", out)
        self.assertIn("P2", out)


# TODO Search with patterns


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
