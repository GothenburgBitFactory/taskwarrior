#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class Test1418(TestCase):
    def setUp(self):
        self.t = Task()

    # Helper methods
    def find_in_list(self, description):
        code, out, err = self.t(("list",))
        self.assertIn(description, out)

    def search_task_pattern(self, description):
        command = ("/" + description.replace("/", "\\/") + "/",)
        code, out, err = self.t(command)
        self.assertIn(description, out)

    def add_search_task(self, description):
        command = ("add", description)
        self.t(command)

    def add_search_task_description(self, description):
        command = ("add", "description:'" + description + "'")
        self.t(command)

    def test_slash_in_description(self):
        """Check that you can search with a slash (/)"""
        description = "foo/"
        self.add_search_task(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_minus_in_description(self):
        """Check that you can search with a minus (-)"""
        description = "foo-"
        self.add_search_task(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_plus_in_description(self):
        """Check that you can search with a plus (+)"""
        description = "foo+"
        self.add_search_task(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_explicit_slash_in_description(self):
        """Can add a task with trailing slash (/) using description:"" """
        description = "foo/"
        self.add_search_task_description(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_explicit_minus_in_description(self):
        """Can add a task with trailing minus (-) using description:"" """
        description = "foo-"
        self.add_search_task_description(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_explicit_plus_in_description(self):
        """Can add a task with trailing plus (+) using description:"" """
        description = "foo+"
        self.add_search_task_description(description)
        self.find_in_list(description)
        self.search_task_pattern(description)

    def test_slash_plus_in_description(self):
        """Can add and search a task with (+) in description"""
        description = "foo+"
        self.add_search_task(description)
        self.find_in_list(description)

        # Different from the other tests, because we want to escape the '+'
        # in the regex, but not in the 'add' or 'list'
        code, out, err = self.t(("/foo\\+/",))
        self.assertIn(description, out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
