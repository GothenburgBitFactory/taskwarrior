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
        """Verify that purge command removes dependency references"""
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


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
