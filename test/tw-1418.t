#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase


class Test1418(TestCase):
    def setUp(self):
        self.t = Task()

    # Helper methods
    def search_task_pattern(self, description):
        # TODO escape any "/" in description - check comments on bug 1418
        command = ("/" + description + "/",)
        code, out, err = self.t(command)
        self.assertIn(description, out)

#    def search_task_bare(self, description):
#        # TODO escape any "/" in description - check comments on bug 1418
#        command = (description,)
#        code, out, err = self.t(command)
#        self.assertIn(description, out)

    def add_search_task(self, description):
        command = ("add", description)
        code, out, err = self.t(command)

    def add_search_task_description(self, description):
        command = ("add", "description:'" + description + "'")
        code, out, err = self.t(command)

    # Tests
#    def test_slash_in_description_bare_words(self):
#        """Check that you can search with a slash (/) and bare words"""
#        description = "foo/"
#        self.add_search_task(description)
#        self.search_task_bare(description)

#    def test_minus_in_description_bare_words(self):
#        """Check that you can search with a minus (-) and bare words"""
#        description = "foo-"
#        self.add_search_task(description)
#        self.search_task_bare(description)

#    def test_plus_in_description_bare_words(self):
#        """Check that you can search with a plus (+) and bare words"""
#        description = "foo+"
#        self.add_search_task(description)
#        self.search_task_bare(description)

#    def test_explicit_slash_in_description_bare_words(self):
#        """Can add a task with trailing slash (/) using description:"" and bare
#        words
#        """
#        description = "foo/"
#        self.add_search_task_description(description)
#        self.search_task_bare(description)

#    def test_explicit_minus_in_description_bare_words(self):
#        """Can add a task with trailing minus (-) using description:"" and bare
#        words
#        """
#        description = "foo-"
#        self.add_search_task_description(description)
#        self.search_task_bare(description)

#    def test_explicit_plus_in_description_bare_words(self):
#        """Can add a task with trailing plus (+) using description:"" and bare
#        words
#        """
#        description = "foo+"
#        self.add_search_task_description(description)
#        self.search_task_bare(description)

    def test_slash_in_description(self):
        """Check that you can search with a slash (/)"""
        description = "foo\\/"
        self.add_search_task(description)
        self.search_task_pattern(description)

    def test_minus_in_description(self):
        """Check that you can search with a minus (-)"""
        self.add_search_task("foo-")
        self.search_task_pattern("foo\\-")

    def test_plus_in_description(self):
        """Check that you can search with a plus (+)"""
        description = "foo\\+"
        self.add_search_task(description)
        self.search_task_pattern(description)

    def test_explicit_slash_in_description(self):
        """Can add a task with trailing slash (/) using description:"" """
        description = "foo\\/"
        self.add_search_task_description(description)
        self.search_task_pattern(description)

    def test_explicit_minus_in_description(self):
        """Can add a task with trailing minus (-) using description:"" """
        self.add_search_task_description("foo-")
        self.search_task_pattern("foo\\-")

    def test_explicit_plus_in_description(self):
        """Can add a task with trailing plus (+) using description:"" """
        description = "foo\\+"
        self.add_search_task_description(description)
        self.search_task_pattern(description)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
