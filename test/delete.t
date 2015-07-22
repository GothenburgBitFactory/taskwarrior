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


class TestDelete(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_add_delete_undo(self):
        """Verify that add/delete/undo yields a Pending task"""
        self.t("add one")
        code, out, err = self.t("_get 1.uuid")
        uuid = out.strip()

        self.t("1 delete")
        self.t.runError("list") # GC/handleRecurrence
        code, out, err = self.t("_get 1.status")
        self.assertEqual("\n", out)

        code, out, err = self.t("_get %s.status" % uuid)
        self.assertIn("deleted\n", out)

        self.t("undo")
        code, out, err = self.t("_get 1.status")
        self.assertIn("pending\n", out)
        code, out, err = self.t("_get %s.status" % uuid)
        self.assertIn("pending\n", out)

    def test_delete_en_passant(self):
        """Verify that en-passant works with delete"""
        self.t("add foo")
        code, out, err = self.t("1 delete project:work")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t("all rc.verbose:nothing")
        self.assertIn("work", out)

    def test_add_done_delete(self):
        """Verify that a completed task can be deleted"""
        self.t("add foo")
        code, out, err = self.t("_get 1.uuid")
        uuid = out.strip()

        code, out, err = self.t("1 done")
        self.assertIn("Completed 1 task.", out)

        self.t("all") # GC/handleRecurrence

        code, out, err = self.t("%s delete" % uuid)
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t("_get %s.status" % uuid)
        self.assertIn("deleted\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
