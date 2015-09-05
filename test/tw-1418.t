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
        code, out, err = self.t("/foo\\+/")
        self.assertIn(description, out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
