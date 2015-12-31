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
import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestFilter(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        cls.t("add project:A prio:H +tag one foo")
        cls.t("add project:A prio:H      two")
        cls.t("add project:A             three")
        cls.t("add           prio:H      four")
        cls.t("add                  +tag five")
        cls.t("add                       six foo")
        cls.t("add           prio:L      seven bar foo")

    def test_list(self):
        """filter - list"""
        code, out, err = self.t("list")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_projectA(self):
        """filter - list project:A"""
        code, out, err = self.t("list project:A")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_priorityH(self):
        """filter - list priority:H"""
        code, out, err = self.t("list priority:H")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_priority(self):
        """filter - list priority:"""
        code, out, err = self.t("list priority:")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_substring(self):
        """filter - list /foo/"""
        code, out, err = self.t("list /foo/")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_double_substring(self):
        """filter - list /foo/ /bar/"""
        code, out, err = self.t("list /foo/ /bar/")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertIn("seven", out)

    def test_list_include_tag(self):
        """filter - list +tag"""
        code, out, err = self.t("list +tag")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_exclude_tag(self):
        """filter - list -tag"""
        code, out, err = self.t("list -tag")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_non_existing_tag(self):
        """filter - list -missing"""
        code, out, err = self.t("list -missing")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_list_mutually_exclusive_tag(self):
        """filter - list +tag -tag"""
        code, out, err = self.t.runError("list +tag -tag")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH(self):
        """filter - list project:A priority:H"""
        code, out, err = self.t("list project:A priority:H")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priority(self):
        """filter - list project:A priority:"""
        code, out, err = self.t("list project:A priority:")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_substring(self):
        """filter - list project:A /foo/"""
        code, out, err = self.t("list project:A /foo/")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_tag(self):
        """filter - list project:A +tag"""
        code, out, err = self.t("list project:A +tag")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_substring(self):
        """filter - list project:A priority:H /foo/"""
        code, out, err = self.t("list project:A priority:H /foo/")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_tag(self):
        """filter - list project:A priority:H +tag"""
        code, out, err = self.t("list project:A priority:H +tag")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_substring_tag(self):
        """filter - list project:A priority:H /foo/ +tag"""
        code, out, err = self.t("list project:A priority:H /foo/ +tag")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_list_projectA_priorityH_substring_tag_substring(self):
        """filter - list project:A priority:H /foo/ +tag /baz/"""
        code, out, err = self.t.runError("list project:A priority:H /foo/ +tag /baz/")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_regex_list_project(self):
        """filter - rc.regex:on list project ~ '[A-Z]'"""
        code, out, err = self.t("rc.regex:on list project ~ \\'[A-Z]\\'")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_regex_list_project_any(self):
        """filter - rc.regex:on list project~."""
        code, out, err = self.t("rc.regex:on list project ~ .")

        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)

    def test_regex_list_substring(self):
        """filter - rc.regex:on list /fo{2}/"""
        code, out, err = self.t("rc.regex:on list /fo{2}/")

        self.assertIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_regex_list_double_substring_wildcard(self):
        """filter - rc.regex:on list /f../ /b../"""
        code, out, err = self.t("rc.regex:on list /f../ /b../")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertIn("seven", out)

    def test_regex_list_substring_startswith(self):
        """filter - rc.regex:on list /^s/"""
        code, out, err = self.t("rc.regex:on list /^s/")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)

    def test_regex_list_substring_wildcard_startswith(self):
        """filter - rc.regex:on list /^.i/"""
        code, out, err = self.t("rc.regex:on list /^.i/")

        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertNotIn("seven", out)

    def test_regex_list_substring_or(self):
        """filter - rc.regex:on list /two|five/"""
        code, out, err = self.t("rc.regex:on list /two|five/")

        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)
        self.assertNotIn("four", out)
        self.assertIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)


class TestFilterDue(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("due",        "4")
        self.t.config("dateformat", "m/d/Y")

        just = datetime.datetime.now() + datetime.timedelta(days=3)
        almost = datetime.datetime.now() + datetime.timedelta(days=5)
        # NOTE: %-m and %-d are unix only
        self.just = just.strftime("%-m/%-d/%Y")
        self.almost = almost.strftime("%-m/%-d/%Y")

    def test_due_filter(self):
        """due tasks filtered correctly"""

        self.t("add one due:{0}".format(self.just))
        self.t("add two due:{0}".format(self.almost))
        self.t("add three due:today")

        code, out, err = self.t("list due:today")
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)

        code, out, err = self.t("list due.is:today")
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)


class TestBug1110(TestCase):
    def setUp(self):
        self.t = Task()

    def test_status_is_case_insensitive(self):
        """filter - status:Completed / status:completed - behave the same"""
        self.t("add ToBeCompleted")
        self.t("1 done")

        code, out, err = self.t("all status:Completed")
        self.assertIn("ToBeCompleted", out)

        code, out, err = self.t("all status:completed")
        self.assertIn("ToBeCompleted", out)


class TestBug480A(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        cls.t.config("defaultwidth", "0")

        cls.t("add one +ordinary")
        cls.t("add two +@strange")

    def test_long_plus_ordinary(self):
        """filter '@' in tags breaks filters: +ordinary"""
        code, out, err = self.t("long +ordinary")
        self.assertIn("one", out)
        self.assertNotIn("two", out)

    def test_long_minus_ordinary(self):
        """filter '@' in tags breaks filters: -ordinary"""
        code, out, err = self.t("long -ordinary")
        self.assertNotIn("one", out)
        self.assertIn("two", out)

    def test_long_plus_at_strange(self):
        """filter '@' in tags breaks filters: +@strange"""
        code, out, err = self.t("long +@strange")
        self.assertNotIn("one", out)
        self.assertIn("two", out)

    def test_long_minus_at_strange(self):
        """filter '@' in tags breaks filters: -@strange"""
        code, out, err = self.t("long -@strange")
        self.assertIn("one", out)
        self.assertNotIn("two", out)


class TestEmptyFilter(TestCase):
    def setUp(self):
        self.t = Task()

    def test_empty_filter_warning(self):
        """Modify tasks with no filter."""

        self.t("add foo")
        self.t("add bar")

        code, out, err = self.t.runError("modify rc.allow.empty.filter=yes rc.confirmation=no priority:H")
        self.assertIn("Command prevented from running.", err)

    def test_empty_filter_error(self):
        """Modify tasks with no filter, and disallowed confirmation."""

        self.t("add foo")
        self.t("add bar")

        code, out, err = self.t.runError("modify rc.allow.empty.filter=no priority:H")
        self.assertIn("You did not specify a filter, and with the 'allow.empty.filter' value, no action is taken.", err)


class TestFilterPrefix(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")

        cls.t('add project:foo.uno  priority:H +tag "one foo"'      )
        cls.t('add project:foo.dos  priority:H      "two"'          )
        cls.t('add project:foo.tres                 "three"'        )
        cls.t('add project:bar.uno  priority:H      "four"'         )
        cls.t('add project:bar.dos             +tag "five"'         )
        cls.t('add project:bar.tres                 "six foo"'      )
        cls.t('add project:bazuno                   "seven bar foo"')
        cls.t('add project:bazdos                   "eight bar foo"')

    def test_list_all(self):
        """No filter shows all tasks."""
        code, out, err = self.t('list')
        self.assertIn('one', out)
        self.assertIn('two', out)
        self.assertIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)

    def test_list_project_foo(self):
        """Filter on project name."""
        code, out, err = self.t('list project:foo')
        self.assertIn('one', out)
        self.assertIn('two', out)
        self.assertIn('three', out)
        self.assertNotIn('four', out)
        self.assertNotIn('five', out)
        self.assertNotIn('six', out)
        self.assertNotIn('seven', out)
        self.assertNotIn('eight', out)

    def test_list_project_not_foo(self):
        """Filter on not project name."""
        code, out, err = self.t('list project.not:foo')
        self.assertNotIn('one', out)
        self.assertNotIn('two', out)
        self.assertNotIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)

    def test_list_project_startswith_bar(self):
        """Filter on project name start."""
        code, out, err = self.t('list project.startswith:bar')
        self.assertNotIn('one', out)
        self.assertNotIn('two', out)
        self.assertNotIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertNotIn('seven', out)
        self.assertNotIn('eight', out)

    def test_list_project_ba(self):
        """Filter on project partial match."""
        code, out, err = self.t('list project:ba')
        self.assertNotIn('one', out)
        self.assertNotIn('two', out)
        self.assertNotIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)

    def test_list_description_has_foo(self):
        """Filter on description pattern."""
        code, out, err = self.t('list description.has:foo')
        self.assertIn('one', out)
        self.assertNotIn('two', out)
        self.assertNotIn('three', out)
        self.assertNotIn('four', out)
        self.assertNotIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)


class TestBug480B(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("defaultwidth", "0")

        self.t("add one +t1")
        self.t("add two +t2")
        self.t("add three +t3")

    def test_numbered_tags(self):
        """filter '-t1 -t2' doesn't work"""
        code, out, err = self.t("list -t1")
        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)

        code, out, err = self.t("list -t1 -t2")
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)

        code, out, err = self.t.runError("list -t1 -t2 -t3")
        self.assertIn("No matches", err)
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)

    def test_numbered_at_tags(self):
        """filter '-t1 -t2' doesn't work when '@' characters are involved"""
        self.t("1 modify +@1")
        self.t("2 modify +@2")
        self.t("3 modify +@3")

        code, out, err = self.t("list -@1")
        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)

        code, out, err = self.t("list -@1 -@2")
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)

        code, out, err = self.t.runError("list -@1 -@2 -@3")
        self.assertIn("No matches", err)
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)

    def test_numbered_at_tags_punctuation(self):
        """filter '-t1 -t2' doesn't work with '@' characters and punctuation"""
        self.t("1 modify +@foo.1")
        self.t("2 modify +@foo.2")
        self.t("3 modify +@foo.3")

        code, out, err = self.t("list -@foo.1")
        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)

        code, out, err = self.t("list -@foo.1 -@foo.2")
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)

        code, out, err = self.t.runError("list -@foo.1 -@foo.2 -@foo.3")
        self.assertIn("No matches", err)
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertNotIn("three", out)


class TestBug485(TestCase):
    @classmethod
    def setUp(cls):
        cls.t = Task()

        cls.t.config("verbose", "nothing")

        cls.t("add one due:tomorrow recur:monthly")
        cls.t("add two due:tomorrow recur:1month")

    def test_filter_recur_monthly(self):
        """filter 'recur:monthly' doesn't list monthly tasks"""
        code, out, err = self.t("list recur:monthly")
        self.assertIn("one", out)
        self.assertIn("two", out)

    def test_filter_recur_1month(self):
        """filter 'recur:1month' doesn't list monthly tasks"""
        code, out, err = self.t("list recur:1month")
        self.assertIn("one", out)
        self.assertIn("two", out)


class TestBug489(TestCase):
    @classmethod
    def setUp(cls):
        cls.t = Task()

    def test_filter_tagless_tasks(self):
        """tags.none: filters tagless tasks"""
        self.t("add with +tag")
        self.t("add without")

        code, out, err = self.t("list tags.none:")
        self.assertNotIn("with ", out)
        self.assertIn("without", out)


class TestBug1600(TestCase):
    def setUp(self):
        self.t = Task()

    def test_filter_plus_in_descriptions(self):
        """filter - description contains +"""
        self.t("add foobar1")
        self.t("add foobar2")
        self.t("add foobar+")

        code, out, err = self.t("all")
        self.assertIn("foobar+", out)
        self.assertIn("foobar1", out)
        self.assertIn("foobar2", out)

        code, out, err = self.t("all description.contains:'foobar\\+'")
        self.assertIn("foobar+", out)
        self.assertNotIn("foobar1", out)
        self.assertNotIn("foobar2", out)

    def test_filter_question_in_descriptions(self):
        """filter - description contains ? """
        self.t("add foobar1")
        self.t("add foo?bar")

        code, out, err = self.t("all")
        self.assertIn("foobar1", out)
        self.assertIn("foo?bar", out)

        code, out, err = self.t("all description.contains:'foo\\?bar'")
        self.assertIn("foo?bar", out)
        self.assertNotIn("foobar1", out)

    def test_filter_brackets_in_descriptions(self):
        """filter - description contains [] """
        self.t("add [foobar1]")
        self.t("add [foobar2]")

        code, out, err = self.t("all")
        self.assertIn("[foobar1]", out)
        self.assertIn("[foobar2]", out)

        code, out, err = self.t("all description.contains:'\\[foobar1\\]'")
        self.assertIn("[foobar1]", out)
        self.assertNotIn("[foobar2]", out)


class TestBug1656(TestCase):
    def setUp(self):
        self.t = Task()

    def test_report_filter_parenthesized(self):
        """default report filter parenthesized"""
        self.t('add task1 +work')
        self.t('add task2 +work')
        self.t('1 done')

        # Sanity check, next does not display completed tasks
        code, out, err = self.t("next")
        self.assertNotIn("task1", out)
        self.assertIn("task2", out)

        # The or in the filter should not cause ignoring the implicit
        # report filter
        code, out, err = self.t("next +home or +work")
        self.assertNotIn("task1", out)
        self.assertIn("task2", out)


class TestRange(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_date_range(self):
        """Verify tasks can be selected by dates ranges"""
        self.t("add one   due:2009-08-01")
        self.t("add two   due:2009-08-03")
        self.t("add three due:2009-08-05")

        code, out, err = self.t("due.after:2009-08-02 due.before:2009-08-05 ls")
        self.assertNotIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)


class TestHasHasnt(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_has_hasnt(self):
        """Verify the 'has' and 'hasnt' attribute modifiers"""
        self.t("add foo")              # 1
        self.t("add foo")              # 2
        self.t("2 annotate bar")
        self.t("add foo")              # 3
        self.t("3 annotate bar")
        self.t("3 annotate baz")
        self.t("add bar")              # 4
        self.t("add bar")              # 5
        self.t("5 annotate foo")
        self.t("add bar")              # 6
        self.t("6 annotate foo")
        self.t("6 annotate baz")
        self.t("add one")              # 7
        self.t("7 annotate two")
        self.t("7 annotate three")

        code, out, err = self.t("description.has:foo long")
        self.assertIn("\n 1", out)
        self.assertIn("\n 2", out)
        self.assertIn("\n 3", out)
        self.assertNotIn("\n 4", out)
        self.assertIn("\n 5", out)
        self.assertIn("\n 6", out)
        self.assertNotIn("\n 7", out)

        code, out, err = self.t("description.hasnt:foo long")
        self.assertNotIn("\n 1", out)
        self.assertNotIn("\n 2", out)
        self.assertNotIn("\n 3", out)
        self.assertIn("\n 4", out)
        self.assertNotIn("\n 5", out)
        self.assertNotIn("\n 6", out)
        self.assertIn("\n 7", out)


class TestBefore(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t('add foo entry:2008-12-22 start:2008-12-22')
        cls.t('add bar entry:2009-04-17 start:2009-04-17')

    def test_correctly_recorded_start(self):
        """Verify start dates properly recorded"""
        code, out, err = self.t("_get 1.start")
        self.assertEqual(out, "2008-12-22T00:00:00\n")

        code, out, err = self.t("_get 2.start")
        self.assertEqual(out, "2009-04-17T00:00:00\n")

    def test_before_none(self):
        """Verify start.before:2008-12-01 yields nothing"""
        code, out, err = self.t("start.before:2008-12-01 _ids")
        self.assertNotIn("1", out)
        self.assertNotIn("2", out)

    def test_after_none(self):
        """Verify start.after:2009-05-01 yields nothing"""
        code, out, err = self.t("start.after:2009-05-01 _ids")
        self.assertNotIn("1", out)
        self.assertNotIn("2", out)

    def test_before_a(self):
        """Verify start.before:2009-01-01 yields '1'"""
        code, out, err = self.t("start.before:2009-01-01 _ids")
        self.assertIn("1", out)
        self.assertNotIn("2", out)

    def test_before_b(self):
        """Verify start.before:2009-05-01 yields '1' and '2'"""
        code, out, err = self.t("start.before:2009-05-01 _ids")
        self.assertIn("1", out)
        self.assertIn("2", out)

    def test_after_a(self):
        """Verify start.after:2008-12-01 yields '1' and '2'"""
        code, out, err = self.t("start.after:2008-12-01 _ids")
        self.assertIn("1", out)
        self.assertIn("2", out)

    def test_after_b(self):
        """Verify start.after:2009-01-01 yields '2'"""
        code, out, err = self.t("start.after:2009-01-01 _ids")
        self.assertNotIn("1", out)
        self.assertIn("2", out)


class Test1424(TestCase):
    def setUp(self):
        self.t = Task()

    def test_1824_days(self):
        """1424: Check that due:1824d works"""
        self.t('add foo due:1824d')
        code, out, err = self.t('_get 1.due.year')
        # NOTE This test has a possible race condition when run "during" EOY.
        # If Taskwarrior is executed at 23:59:59 on new year's eve and the
        # python code below runs at 00:00:00 on new year's day, the two will
        # disagree on the proper year. Using libfaketime with a frozen time
        # or the date set to $year-01-01 might be a good idea here.
        plus_1824d = datetime.datetime.today() + datetime.timedelta(days=1824)
        self.assertEqual(out, "%d\n" % (plus_1824d.year))

    def test_3648_days(self):
        """1424: Check that due:3648d works"""
        self.t('add foo due:3648d')
        code, out, err = self.t('_get 1.due.year')
        # NOTE This test has a possible race condition when run "during" EOY.
        # If Taskwarrior is executed at 23:59:59 on new year's eve and the
        # python code below runs at 00:00:00 on new year's day, the two will
        # disagree on the proper year. Using libfaketime with a frozen time
        # or the date set to $year-01-01 might be a good idea here.
        plus_3648d = datetime.datetime.today() + datetime.timedelta(days=3648)
        self.assertEqual(out, "%d\n" % (plus_3648d.year))


# TODO This does not look right, it adds one task, exports it, and checks the UUID.
#      The 'uuid:' filter could be ignored, and this test might pass.
class Test1452(TestCase):
    def setUp(self):
        self.t = Task()
        self.t('add task')
        self.task_uuid = self.t.export_one()['uuid']

    def test_get_task_by_uuid_with_prefix(self):
        """1452: Tries to filter task simply by it's uuid, using uuid: prefix."""
        output = self.t.export_one('uuid:%s' % self.task_uuid)

        # Sanity check it is the correct one
        self.assertEqual(output['uuid'], self.task_uuid)

    def test_get_task_by_uuid_without_prefix(self):
        """1452: Tries to filter task simply by it's uuid, without using uuid: prefix."""
        output = self.t.export_one(self.task_uuid)

        # Sanity check it is the correct one
        self.assertEqual(output['uuid'], self.task_uuid)


class TestBug1456(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_quoted_expressions(self):
        """1456: Verify that a multi-term quoted filter expression works"""
        self.t("add zero")
        self.t("add one")
        self.t("add two")

        code, out, err = self.t("'/one/ or /two/' list")
        self.assertNotIn("zero", out)
        self.assertIn("one", out)
        self.assertIn("two", out)


class Test1468(TestCase):
    def setUp(self):
        self.t = Task()
        self.t('add project:home buy milk')
        self.t('add project:home mow the lawn')

    def test_single_attribute_filter(self):
        """1468: Single attribute filter (project:home)"""
        code, out, err = self.t('list project:home')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('buy milk', out)
        self.assertIn('mow the lawn', out)

    def test_attribute_and_search_filter(self):
        """1468: Attribute and implicit search filter (project:home /lawn/)"""
        code, out, err = self.t('list project:home /lawn/')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertNotIn('buy milk', out)
        self.assertIn('mow the lawn', out)


class TestBug1521(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")
        cls.t("add one project:WORK")
        cls.t("add two project:HOME")

    def setUp(self):
        """Executed before each test in the class"""

    def test_project_inequality(self):
        """1521: Verify that 'project.not' works"""
        code, out, err = self.t("project.not:WORK list")
        self.assertNotIn("one", out)
        self.assertIn("two", out)

    def test_project_not_equal(self):
        """1521: Verify that 'project !=' works"""
        code, out, err = self.t("project != WORK list")
        self.assertNotIn("one", out)
        self.assertIn("two", out)


class TestBug1609(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_urgency_comparison(self):
        """1609: Test that urgency is filterable"""
        self.t("add one project:P due:yesterday +tag1 +tag2")
        self.t("add two")

        code, out, err = self.t("'urgency<10' next")
        self.assertNotIn("one", out)
        self.assertIn("two", out)


class TestBug1630(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add zero")
        self.t("add one due:7d")
        self.t("add two due:10d")

    def test_attribute_modifier_with_duration(self):
        """1630: Verify that 'due.before:9d' is correctly interpreted"""
        code, out, err = self.t("due.before:9d list rc.verbose:nothing")
        self.assertNotIn("zero", out)
        self.assertIn("one", out)
        self.assertNotIn("two", out)

    def test_attribute_no_modifier_with_duration(self):
        """1630: Verify that 'due:7d' is correctly interpreted"""
        code, out, err = self.t("due:7d list rc.verbose:nothing")
        self.assertNotIn("zero", out)
        self.assertIn("one", out)
        self.assertNotIn("two", out)


class Test1634(TestCase):
    def setUp(self):
        self.t = Task()

        # Setup some tasks due on 2015-07-07
        self.t('add due:2015-07-07T00:00:00 ON1')
        self.t('add due:2015-07-07T14:34:56 ON2')
        self.t('add due:2015-07-07T23:59:59 ON3')

        # Setup some tasks not due on 2015-07-07
        self.t('add due:2015-07-06T23:59:59 OFF4')
        self.t('add due:2015-07-08T00:00:00 OFF5')
        self.t('add due:2015-07-08T00:00:01 OFF6')
        self.t('add due:2015-07-06T00:00:00 OFF7')

    def test_due_match_not_exact(self):
        """1634: Test that due:<date> matches any task that date."""
        code, out, err = self.t('due:2015-07-07 minimal')

        # Asswer that only tasks ON the date are listed.
        self.assertIn("ON1", out)
        self.assertIn("ON2", out)
        self.assertIn("ON3", out)

        # Assert that tasks on other dates are not listed.
        self.assertNotIn("OFF4", out)
        self.assertNotIn("OFF5", out)
        self.assertNotIn("OFF6", out)
        self.assertNotIn("OFF7", out)

    def test_due_not_match_not_exact(self):
        """1634: Test that due.not:<date> does not match any task that date."""
        code, out, err = self.t('due.not:2015-07-07 minimal')

        # Assert that task ON the date are not listed.
        self.assertNotIn("ON1", out)
        self.assertNotIn("ON2", out)
        self.assertNotIn("ON3", out)

        # Assert that tasks on other dates are listed.
        self.assertIn("OFF4", out)
        self.assertIn("OFF5", out)
        self.assertIn("OFF6", out)
        self.assertIn("OFF7", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
