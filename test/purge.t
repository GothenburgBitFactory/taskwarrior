#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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


class TestDelete(TestCase):
    def setUp(self):
        self.t = Task()

    def test_add_delete_purge(self):
        """Verify that add/delete/purge successfully purges a task"""
        self.t("add one")
        uuid = self.t("_get 1.uuid")[1].strip()

        code, out, err = self.t("1 delete", input="y\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t(uuid + " purge", input="y\n")
        self.assertIn("Purged 1 task.", out)

        code, out, err = self.t("uuids")
        self.assertNotIn(uuid, out)

    def test_purge_remove_deps(self):
        """Purge command removes broken dependency references"""
        self.t("add one")
        self.t("add two dep:1")
        uuid = self.t("_get 1.uuid")[1].strip()

        code, out, err = self.t("1 delete", input="y\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t(uuid + " purge", input="y\n")
        self.assertIn("Purged 1 task.", out)

        code, out, err = self.t("uuids")
        self.assertNotIn(uuid, out)

        dependencies = self.t("_get 1.depends")[1].strip()
        self.assertNotIn(uuid, dependencies)

    def test_purge_children(self):
        """Purge command indirectly purges child tasks"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")

        code, out, err = self.t("1 delete", input="y\ny\n")
        self.assertIn("Deleted 4 tasks.", out)

        code, out, err = self.t(uuid + " purge", input="y\ny\n")
        self.assertIn("Purged 4 tasks.", out)

        code, out, err = self.t("uuids")
        self.assertEqual('\n', out)

    def test_purge_children_fail_pending(self):
        """Purge aborts if task has pending children"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")

        code, out, err = self.t("1 delete", input="y\nn\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t.runError(uuid + " purge", input="y\n")
        self.assertIn("child task 1 must be deleted before", err)

        # Check that nothing was purged
        code, out, err = self.t("count")
        self.assertEqual('4\n', out)

    def test_purge_children_fail_confirm(self):
        """Purge aborts if user does not agree with it affecting child tasks"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")

        code, out, err = self.t("1 delete", input="y\ny\n")
        self.assertIn("Deleted 4 tasks.", out)

        # Do not agree with purging of the child tasks
        code, out, err = self.t.runError(uuid + " purge", input="y\nn\n")
        self.assertIn("Purge operation aborted.", err)

        # Check that nothing was purged
        code, out, err = self.t("count")
        self.assertEqual('4\n', out)

    def test_purge_children(self):
        """Purge command removes dependencies on indirectly purged tasks"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")
        self.t("add two dep:4")

        # Check that the dependency is present
        dependencies = self.t("_get 5.depends")[1].strip()
        self.assertNotEqual("", dependencies)

        code, out, err = self.t("1 delete", input="y\ny\n")
        self.assertIn("Deleted 4 tasks.", out)

        code, out, err = self.t(uuid + " purge", input="y\ny\n")
        self.assertIn("Purged 4 tasks.", out)

        # Make sure we are dealing with the intended task
        description = self.t("_get 1.description")[1].strip()
        self.assertEqual("two", description)

        # Check that the dependency was removed
        dependencies = self.t("_get 1.depends")[1].strip()
        self.assertEqual("", dependencies)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
