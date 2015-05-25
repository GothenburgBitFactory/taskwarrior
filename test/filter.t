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


class TestFilter(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        cls.t(("add", "project:A", "prio:H", "+tag", "one", "foo"))
        cls.t(("add", "project:A", "prio:H",         "two"))
        cls.t(("add", "project:A",                   "three"))
        cls.t(("add",              "prio:H",         "four"))
        cls.t(("add",                        "+tag", "five"))
        cls.t(("add",                                "six", "foo"))
        cls.t(("add",              "prio:L",         "seven", "bar", "foo"))

    def test_list(self):
        """filter - list"""
        code, out, err = self.t(("list",))

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_projectA(self):
        """filter - list project:A"""
        code, out, err = self.t(("list", "project:A"))

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_priorityH(self):
        """filter - list priority:H"""
        code, out, err = self.t(("list", "priority:H"))

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_priority(self):
        """filter - list priority:"""
        code, out, err = self.t(("list", "priority:"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_substring(self):
        """filter - list /foo/"""
        code, out, err = self.t(("list", "/foo/"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_double_substring(self):
        """filter - list /foo/ /bar/"""
        code, out, err = self.t(("list", "/foo/", "/bar/"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertIn("seven", out)

    def test_list_include_tag(self):
        """filter - list +tag"""
        code, out, err = self.t(("list", "+tag"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_exclude_tag(self):
        """filter - list -tag"""
        code, out, err = self.t(("list", "-tag"))

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_non_existing_tag(self):
        """filter - list -missing"""
        code, out, err = self.t(("list", "-missing"))

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_mutually_exclusive_tag(self):
        """filter - list +tag -tag"""
        code, out, err = self.t.runError(("list", "+tag", "-tag"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH(self):
        """filter - list project:A priority:H"""
        code, out, err = self.t(("list", "project:A", "priority:H"))

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priority(self):
        """filter - list project:A priority:"""
        code, out, err = self.t(("list", "project:A", "priority:"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_substring(self):
        """filter - list project:A /foo/"""
        code, out, err = self.t(("list", "project:A", "/foo/"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_tag(self):
        """filter - list project:A +tag"""
        code, out, err = self.t(("list", "project:A", "+tag"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_substring(self):
        """filter - list project:A priority:H /foo/"""
        code, out, err = self.t(("list", "project:A", "priority:H", "/foo/"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_tag(self):
        """filter - list project:A priority:H +tag"""
        code, out, err = self.t(("list", "project:A", "priority:H", "+tag"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_substring_tag(self):
        """filter - list project:A priority:H /foo/ +tag"""
        code, out, err = self.t(("list", "project:A", "priority:H", "/foo/",
                                 "+tag"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_substring_tag_substring(self):
        """filter - list project:A priority:H /foo/ +tag /baz/"""
        code, out, err = self.t.runError(("list", "project:A", "priority:H",
                                          "/foo/", "+tag", "/baz/"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    @unittest.expectedFailure
    def test_regex_list_project(self):
        """filter - rc.regex:on list project:/[A-Z]/"""
        code, out, err = self.t(("rc.regex:on", "list", "project:/[A-Z]/"))

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    @unittest.expectedFailure
    def test_regex_list_project_any(self):
        """filter - rc.regex:on list project:."""
        code, out, err = self.t(("rc.regex:on", "list", "project:."))

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_regex_list_substring(self):
        """filter - rc.regex:on list /fo{2}/"""
        code, out, err = self.t(("rc.regex:on", "list", "/fo{2}/"))

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_regex_list_double_substring_wildcard(self):
        """filter - rc.regex:on list /f../ /b../"""
        code, out, err = self.t(("rc.regex:on", "list", "/f../", "/b../"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertIn("seven", out)

    def test_regex_list_substring_startswith(self):
        """filter - rc.regex:on list /^s/"""
        code, out, err = self.t(("rc.regex:on", "list", "/^s/"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_regex_list_substring_wildcard_startswith(self):
        """filter - rc.regex:on list /^.i/"""
        code, out, err = self.t(("rc.regex:on", "list", "/^.i/"))

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertNotIn("seven", out)

    def test_regex_list_substring_or(self):
        """filter - rc.regex:on list /two|five/"""
        code, out, err = self.t(("rc.regex:on", "list", "/two|five/"))

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)


class TestBug1110(TestCase):
    def setUp(self):
        self.t = Task()

    def test_status_is_case_insensitive(self):
        """filter - status:Completed / status:completed - behave the same"""
        self.t(("add", "ToBeCompleted"))
        self.t(("1", "done"))

        code, out, err = self.t(("all", "status:Completed"))
        self.assertIn("ToBeCompleted", out)

        code, out, err = self.t(("all", "status:completed"))
        self.assertIn("ToBeCompleted", out)


@unittest.skip("WaitingFor TW-1600")
class TestBug1600(TestCase):
    def setUp(self):
        self.t = Task()

    def test_filter_plus_in_descriptions(self):
        """filter - description contains +"""
        self.t(("add", "foobar1"))
        self.t(("add", "foobar2"))
        self.t(("add", "foobar+"))

        code, out, err = self.t(("all",))
        self.assertIn("foobar+", out)
        self.assertIn("foobar1", out)
        self.assertIn("foobar2", out)

        code, out, err = self.t(("all", "description.contains:'foobar+'"))
        self.assertIn("foobar+", out)
        self.assertNotIn("foobar1", out)
        self.assertNotIn("foobar2", out)

    def test_filter_question_in_descriptions(self):
        """filter - description contains ? """
        self.t(("add", "foobar1"))
        self.t(("add", "foo?bar"))

        code, out, err = self.t(("all",))
        self.assertIn("foobar1", out)
        self.assertIn("foo?bar", out)

        code, out, err = self.t(("all", "description.contains:'foo?bar'"))
        self.assertIn("foo?bar", out)
        self.assertNotIn("foobar1", out)

    def test_filter_brackets_in_descriptions(self):
        """filter - description contains [] """
        self.t(("add", "[foobar1]"))
        self.t(("add", "[foobar2]"))

        code, out, err = self.t(("all",))
        self.assertIn("[foobar1]", out)
        self.assertIn("[foobar2]", out)

        code, out, err = self.t(("all", "description.contains:'[foobar]'"))
        self.assertIn("[foobar1]", out)
        self.assertNotIn("[foobar2]", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
